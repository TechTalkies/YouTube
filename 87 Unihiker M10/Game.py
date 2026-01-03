# -*- coding: utf-8 -*-
# UNIHIKER M10 – Touch Controlled Auto-Fire Shooter
# Resolution: 240x320
# Drag to move, auto fire, premium minimal visuals

import tkinter as tk
import random
import time

# ----------------------------
# Display config
# ----------------------------
WIDTH = 240
HEIGHT = 320
BG = "#0b0e14"
SHIP_COLOR = "#2d89ef"
BULLET_COLOR = "#00ffcc"
ENEMY_COLOR = "#ff4d4d"

# ----------------------------
# Game parameters
# ----------------------------
SHIP_SIZE = 14
BULLET_SPEED = 6
ENEMY_SPEED = 2
FIRE_RATE = 250   # ms
ENEMY_RATE = 900  # ms

# ----------------------------
# App
# ----------------------------
root = tk.Tk()
root.title("UNIHIKER Shooter")
root.geometry(f"{WIDTH}x{HEIGHT}")
root.configure(bg=BG)
root.resizable(False, False)

canvas = tk.Canvas(
    root,
    width=WIDTH,
    height=HEIGHT,
    bg=BG,
    highlightthickness=0
)
canvas.pack()

# ----------------------------
# Player ship
# ----------------------------
ship_x = WIDTH // 2
ship_y = HEIGHT - 40

ship = canvas.create_polygon(
    ship_x, ship_y - SHIP_SIZE,
    ship_x - SHIP_SIZE, ship_y + SHIP_SIZE,
    ship_x + SHIP_SIZE, ship_y + SHIP_SIZE,
    fill=SHIP_COLOR
)

# ----------------------------
# Game state
# ----------------------------
bullets = []
enemies = []
last_fire = 0
last_enemy = 0

# ----------------------------
# Touch / drag control
# ----------------------------
def on_drag(event):
    global ship_x, ship_y
    ship_x = max(20, min(WIDTH - 20, event.x))
    ship_y = max(40, min(HEIGHT - 20, event.y))

    canvas.coords(
        ship,
        ship_x, ship_y - SHIP_SIZE,
        ship_x - SHIP_SIZE, ship_y + SHIP_SIZE,
        ship_x + SHIP_SIZE, ship_y + SHIP_SIZE
    )

canvas.bind("<B1-Motion>", on_drag)
canvas.bind("<Button-1>", on_drag)

# ----------------------------
# Bullet logic
# ----------------------------
def fire_bullet():
    bullet = canvas.create_rectangle(
        ship_x - 2, ship_y - 10,
        ship_x + 2, ship_y,
        fill=BULLET_COLOR,
        outline=""
    )
    bullets.append(bullet)

# ----------------------------
# Enemy logic
# ----------------------------
def spawn_enemy():
    x = random.randint(20, WIDTH - 20)
    enemy = canvas.create_oval(
        x - 10, -20,
        x + 10, 0,
        fill=ENEMY_COLOR,
        outline=""
    )
    enemies.append(enemy)

# ----------------------------
# Collision check
# ----------------------------
def overlap(a, b):
    ax1, ay1, ax2, ay2 = canvas.bbox(a)
    bx1, by1, bx2, by2 = canvas.bbox(b)
    return not (ax2 < bx1 or ax1 > bx2 or ay2 < by1 or ay1 > by2)

# ----------------------------
# Game loop
# ----------------------------
def game_loop():
    global last_fire, last_enemy

    now = int(time.time() * 1000)

    # Auto fire
    if now - last_fire > FIRE_RATE:
        fire_bullet()
        last_fire = now

    # Spawn enemies
    if now - last_enemy > ENEMY_RATE:
        spawn_enemy()
        last_enemy = now

    # Move bullets
    for bullet in bullets[:]:
        canvas.move(bullet, 0, -BULLET_SPEED)
        if canvas.bbox(bullet)[3] < 0:
            canvas.delete(bullet)
            bullets.remove(bullet)

    # Move enemies
    for enemy in enemies[:]:
        canvas.move(enemy, 0, ENEMY_SPEED)
        if canvas.bbox(enemy)[1] > HEIGHT:
            canvas.delete(enemy)
            enemies.remove(enemy)

    # Collision detection
    for bullet in bullets[:]:
        for enemy in enemies[:]:
            if overlap(bullet, enemy):
                canvas.delete(bullet)
                canvas.delete(enemy)
                bullets.remove(bullet)
                enemies.remove(enemy)
                break

    root.after(16, game_loop)

# ----------------------------
# Start
# ----------------------------
game_loop()
root.mainloop()
