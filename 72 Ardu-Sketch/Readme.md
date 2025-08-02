# Ardu-Sketch 
An Arduino Nano–powered version on the classic Etch A Sketch.  
Built with rotary encoders, a tiny OLED screen, and a lot of retro charm.

[Full tutorial video link](https://youtu.be/xDhLpARkTvY)

![BYTE-A-SKETCH Demo](demo.gif)

## ✨ What It Is  
BYTE-A-SKETCH lets you draw pixel art using two rotary encoders — one for X and one for Y movement — just like the iconic toy, but digital.  
It’s oddly satisfying, incredibly simple, and perfect for beginners or nostalgia nerds.

## 🛠️ Hardware Used  
- Arduino Nano  
- 0.96" 128x64 OLED (SSD1306)  
- 2x Rotary Encoders (with push buttons)  
- Breadboard + jumper wires  

## 🔧 Features  
- Draw using encoder knobs
- Change the brush size
- Press left knob to clear screen  (with confirmation prompt)
  

## 📦 Wiring

| Component        | Arduino Pin     |
|------------------|------------------|
| OLED SDA         | A4               |
| OLED SCL         | A5               |
| Left Encoder CLK | D5               |
| Left Encoder DT  | D4               |
| Left Button      | D3               |
| Right Encoder CLK| D8               |
| Right Encoder DT | D7               |
| Right Button     | D6               |

> 🟡 Note: Adjust pins in code if needed

## 🧠 How It Works  
Each encoder controls a direction — left knob moves vertically, right moves horizontally.  
You “draw” pixel by pixel as you move the knobs, and can clear the canvas at any time.  
Simple logic + satisfying feedback.

## 🚀 Getting Started

1. Dwonload this repo  
2. Install [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) and [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library) libraries  
3. Upload sketch to your Nano  
4. Power it up, twist the knobs, and draw away!

## 🧹 To Do
- Add save/load feature via EEPROM or SD  
- Export drawings as bitmap over Serial  
- Add multi-pattern drawing modes

## 🎥 Demo Video  
Check out the build and full explanation on [YouTube](https://youtube.com/yourchannel).

---

Made with love and rotary encoders ❤️  
