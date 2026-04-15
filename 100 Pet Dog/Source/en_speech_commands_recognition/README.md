# ESP32-S3 Speech Controlled Robot Dog

## 🧠 Base Framework

This project is built on top of Espressif's official ESP-Skainet example:

https://github.com/espressif/esp-skainet

All credit for the speech recognition framework goes to Espressif.


## ⚙️ Board Configuration

This project is preconfigured for the XIAO ESP32S3 Sense.

> ⚠️ Important:  
> The target is not automatically selected. You must set it manually before building.

```bash
idf.py set-target esp32s3
```
Or use the menu at the bottom.

## 🚀 Build Instructions

Make a full clean before building and flashing the code.

## 🎯 Commands

Example recognized commands:

- Sit  
- Sit down  
- Come here  
- Walk  
- Dance  
- Stretch

More commands can be added in the ESP IDF config menu. These commands can be generated using the tool here:

https://github.com/espressif/esp-sr/tree/66e21f6cc384d6b4aec077c187ebb0f5fbb4c5ff/speech_command_recognition/tool

## ⚠️ Disclaimer

This is a simplified implementation intended for demonstration and experimentation.  
Speech recognition accuracy depends on environment, microphone quality, and model configuration.
