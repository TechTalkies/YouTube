import psutil
import subprocess
import time
import threading
import socket
import tkinter as tk
from tkinter import ttk

import pystray
from PIL import Image, ImageDraw


SEND_INTERVAL = 1.0  # seconds

# ---------- Network tracking ----------
prev_net = psutil.net_io_counters()
prev_net_time = time.time()


def get_gpu_usage():
    try:
        result = subprocess.check_output(
            [
                "nvidia-smi",
                "--query-gpu=utilization.gpu",
                "--format=csv,noheader,nounits"
            ],
            stderr=subprocess.DEVNULL
        ).decode().strip()

        return int(result)

    except Exception:
        return -1


def get_network_activity():
    global prev_net, prev_net_time

    now = time.time()
    curr = psutil.net_io_counters()

    dt = now - prev_net_time
    if dt <= 0:
        return 0, 0, 0

    down = int((curr.bytes_recv - prev_net.bytes_recv) / dt)
    up = int((curr.bytes_sent - prev_net.bytes_sent) / dt)

    total_bps = down + up

    MAX_BYTES_PER_SEC = 512 * 1024  # 512 KB/s
    percent = int((total_bps / MAX_BYTES_PER_SEC) * 100)
    percent = max(0, min(100, percent))

    prev_net = curr
    prev_net_time = now

    return percent, down, up


def create_tray_icon_image():
    img = Image.new("RGB", (64, 64), (30, 30, 30))
    d = ImageDraw.Draw(img)
    d.rectangle((16, 16, 48, 48), fill=(80, 180, 120))
    return img


class MonitorApp:
    def __init__(self, root):
        self.root = root
        self.root.title("ESP32 Hardware Monitor")

        self.running = False
        self.sock = None
        self.thread = None

        self.ip_var = tk.StringVar(value="192.168.1.27")
        self.port_var = tk.IntVar(value=5000)
        self.status_var = tk.StringVar(value="Idle")

        self.tray_icon = None

        self.build_ui()
        self.setup_tray()

    # ---------- UI ----------

    def build_ui(self):
        frame = ttk.Frame(self.root, padding=10)
        frame.grid()

        ttk.Label(frame, text="ESP32 IP").grid(row=0, column=0, sticky="w")
        ttk.Entry(frame, textvariable=self.ip_var, width=16).grid(row=0, column=1)

        ttk.Label(frame, text="Port").grid(row=0, column=2, padx=(10, 0))
        ttk.Entry(frame, textvariable=self.port_var, width=6).grid(row=0, column=3)

        self.start_btn = ttk.Button(frame, text="Start", command=self.toggle)
        self.start_btn.grid(row=1, column=0, columnspan=4, sticky="ew", pady=8)

        self.quit_btn = ttk.Button(frame, text="Quit", command=self.quit_app)
        self.quit_btn.grid(row=2, column=0, columnspan=4, sticky="ew")

        # connection status (separate)
        ttk.Label(frame, textvariable=self.status_var).grid(
            row=3, column=0, columnspan=4, pady=(6, 6)
        )

        # ---- latest sent packet ----
        ttk.Label(frame, text="Last sent packet").grid(
            row=4, column=0, columnspan=4, sticky="w"
        )

        self.packet_box = tk.Text(
            frame,
            width=48,
            height=3,
            state="disabled",
            font=("Consolas", 10),
            wrap="none"
        )
        self.packet_box.grid(row=5, column=0, columnspan=4, sticky="ew")

    # ---------- Tray ----------

    def setup_tray(self):
        menu = pystray.Menu(
            pystray.MenuItem("Show", self.tray_show, default=True),
            pystray.MenuItem("Quit", self.tray_quit)
        )

        self.tray_icon = pystray.Icon(
            "ESP32 Monitor",
            create_tray_icon_image(),
            "ESP32 Hardware Monitor",
            menu
        )

        self.tray_icon.on_activate = self.tray_double_click
        threading.Thread(target=self.tray_icon.run, daemon=True).start()

        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def tray_show(self, icon=None, item=None):
        self.root.after(0, self.root.deiconify)

    def tray_double_click(self, icon, item):
        self.tray_show()

    def tray_quit(self, icon, item):
        self.quit_app()

    def on_close(self):
        self.root.withdraw()

    # ---------- Control ----------

    def toggle(self):
        if not self.running:
            self.start()
        else:
            self.stop()

    def start(self):
        ip = self.ip_var.get()
        port = self.port_var.get()

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5)
            self.sock.connect((ip, port))
            self.sock.settimeout(None)
        except Exception as e:
            self.status_var.set(f"Connect failed: {e}")
            return

        self.running = True
        self.start_btn.config(text="Stop")
        self.status_var.set("Connected")
        self.thread = threading.Thread(target=self.worker, daemon=True)
        self.thread.start()

    def stop(self):
        self.running = False
        self.start_btn.config(text="Start")
        self.status_var.set("Stopped")

        if self.sock:
            try:
                self.sock.close()
            except Exception:
                pass
            self.sock = None

    def quit_app(self):
        self.stop()
        if self.tray_icon:
            self.tray_icon.stop()
        self.root.after(0, self.root.destroy)

    # ---------- UI helper ----------

    def update_packet_box(self, text):
        self.packet_box.config(state="normal")
        self.packet_box.delete("1.0", "end")
        self.packet_box.insert("end", text.strip())
        self.packet_box.config(state="disabled")

    # ---------- Worker ----------

    def worker(self):
        while self.running:
            cpu = int(psutil.cpu_percent(interval=None))
            ram = int(psutil.virtual_memory().percent)
            gpu = get_gpu_usage()
            net, down, up = get_network_activity()

            packet = (
                f"CPU={cpu};RAM={ram};GPU={gpu};"
                f"NET={net};DOWN={down};UP={up}\n"
            )

            try:
                self.sock.sendall(packet.encode("ascii"))
                self.root.after(0, self.update_packet_box, packet)
            except Exception:
                self.root.after(0, self.status_var.set, "Connection lost")
                self.stop()
                break

            time.sleep(SEND_INTERVAL)


if __name__ == "__main__":
    root = tk.Tk()
    app = MonitorApp(root)
    root.mainloop()
