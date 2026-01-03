import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext
from pygments import lex
from pygments.lexers import PythonLexer
from pygments.styles import get_style_by_name

# ----------------------------
# Config
# ----------------------------
STYLE = get_style_by_name("monokai")

BG = "#121212"
FG = "#e0e0e0"
BTN = "#1f1f1f"
ACCENT = "#2d89ef"
KEY_BG = "#2a2a2a"
KEY_FG = "#ffffff"

# ----------------------------
# Syntax highlight
# ----------------------------
def highlight_syntax(text_widget):
    code = text_widget.get("1.0", tk.END)
    text_widget.tag_delete(*text_widget.tag_names())

    index = "1.0"
    for token, content in lex(code, PythonLexer()):
        tag = str(token)
        text_widget.tag_config(tag, foreground=STYLE.styles.get(token, FG))
        end = f"{index}+{len(content)}c"
        text_widget.tag_add(tag, index, end)
        index = end

# ----------------------------
# App
# ----------------------------
class MicroPythonEditor(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Portable Python Editor")
        self.geometry("800x480")
        self.configure(bg=BG)

        self.file_path = None
        self.keyboard_visible = False

        self.create_ui()
        self.bind("<Configure>", self.on_resize)

    # ----------------------------
    def create_ui(self):
        # Toolbar
        bar = tk.Frame(self, bg=BG)
        bar.pack(fill="x", padx=6, pady=4)

        def btn(txt, cmd):
            return tk.Button(
                bar, text=txt, command=cmd,
                bg=BTN, fg=FG, activebackground=ACCENT,
                relief="flat", height=2, padx=10
            )

        btn("Open", self.open_file).pack(side="left", padx=4)
        btn("Save", self.save_file).pack(side="left", padx=4)
        btn("⌨", self.toggle_keyboard).pack(side="left", padx=4)
        
        # Editor
        self.editor = scrolledtext.ScrolledText(
            self,
            font=("Courier", 12),
            bg=BG,
            fg=FG,
            insertbackground=FG
        )
        self.editor.pack(fill="both", expand=True, padx=6, pady=4)
        self.editor.bind("<KeyRelease>", lambda e: highlight_syntax(self.editor))

        # Keyboard overlay frame (initially hidden)
        self.keyboard_frame = tk.Frame(self, bg=BG)
        
        '''
        # Floating keyboard button
        self.kb_btn = tk.Button(
            self,
            text="⌨",
            command=self.toggle_keyboard,
            bg=ACCENT,
            fg="white",
            relief="flat",
            font=("Arial", 14),
            width=3,
            height=1
        )
        self.kb_btn.place(relx=1.0, rely=1.0, x=-20, y=-20, anchor="se")
        '''

        self.build_keyboard()

    # ----------------------------
    def build_keyboard(self):
        layout = [
            ["1","2","3","4","5","6","7","8","9","0"],
            ["q","w","e","r","t","y","u","i","o","p"],
            ["a","s","d","f","g","h","j","k","l"],
            ["z","x","c","v","b","n","m","space","back"]
        ]

        for row in layout:
            row_frame = tk.Frame(self.keyboard_frame, bg=BG)
            row_frame.pack(pady=4)

            for key in row:
                label = "␣" if key == "space" else ("⌫" if key == "back" else key)
                w = 1 if key == "space" else 1

                tk.Button(
                    row_frame,
                    text=label,
                    width=w,
                    height=1,
                    padx=4,
                    pady=0,
                    bg=KEY_BG,
                    fg=KEY_FG,
                    relief="flat",
                    command=lambda k=key: self.key_press(k)
                ).pack(side="left", padx=0)

    # ----------------------------
    def key_press(self, key):
        if key == "space":
            self.editor.insert(tk.INSERT, " ")
        elif key == "back":
            self.editor.delete("insert-1c")
        else:
            self.editor.insert(tk.INSERT, key)

    # ----------------------------
    def toggle_keyboard(self):
        if self.keyboard_visible:
            self.keyboard_frame.place_forget()
            self.keyboard_visible = False
        else:
            self.keyboard_visible = True
            self.resize_keyboard()

    def on_resize(self, event):
        if self.keyboard_visible:
            self.resize_keyboard()

    def resize_keyboard(self):
        total_h = self.winfo_height()
        total_w = self.winfo_width()
        kb_h = int(total_h * 0.5)

        self.keyboard_frame.place(
            x=0,
            y=total_h - kb_h,
            width=total_w,
            height=kb_h
        )

    # ----------------------------
    # File ops
    # ----------------------------
    def open_file(self):
        path = filedialog.askopenfilename(filetypes=[("Python", "*.py")])
        if not path:
            return
        self.file_path = path
        with open(path) as f:
            self.editor.delete("1.0", tk.END)
            self.editor.insert(tk.END, f.read())
        highlight_syntax(self.editor)

    def save_file(self):
        if not self.file_path:
            path = filedialog.asksaveasfilename(defaultextension=".py")
            if not path:
                return
            self.file_path = path

        with open(self.file_path, "w") as f:
            f.write(self.editor.get("1.0", tk.END))

        messagebox.showinfo("Saved", "File saved")

# ----------------------------
if __name__ == "__main__":
    MicroPythonEditor().mainloop()