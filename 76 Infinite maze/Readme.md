# Arduino / ESP32 OLED Maze Generator

This project demonstrates how to generate **infinite random mazes** on a microcontroller using an OLED display.  
The maze is generated with the **recursive backtracking algorithm** and visualized step by step on the screen.

## Features
- Works with **ESP32** or **Arduino** boards  
- Supports **SH1106 OLED** displays (using the U8g2 library)  
- Step-by-step maze visualization  
- Infinite maze levels, generated procedurally  

## Requirements
- ESP32 or Arduino board  
- 1.3" OLED Display (SH1106, I2C)  
- [U8g2 library](https://github.com/olikraus/u8g2)  

## How It Works
The algorithm "carves" passages through a grid one cell at a time.  
You can actually see the maze being created in real-time on the OLED screen.

## Usage
1. Download this repository  
2. Open the code in Arduino IDE / PlatformIO  
3. Select your board and upload the sketch  

## Demo
The display shows the maze being carved step by step, creating a new random maze every run.
