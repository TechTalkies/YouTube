# ESP32 Flight Radar - 7 Inch

A real-time Flight Radar built using the **Elecrow 7-inch CrowPanel Advance HMI Display**. The project downloads live aircraft data from the **OpenSky Network API** and displays nearby aircraft on an animated radar with a touchscreen interface.

Video tutorial:

[![Youtube Video](https://img.youtube.com/vi/_Vu7CyQgKxw/0.jpg)](https://www.youtube.com/watch?v=_Vu7CyQgKxw)

## Features

- ✈️ Live aircraft tracking using the OpenSky Network API
- 📡 Animated radar sweep
- 🎯 Aircraft position prediction between API updates
- 📱 7-inch capacitive touchscreen interface
- 🛩️ Previous/Next aircraft selection
- ⭐ Selected aircraft highlighting
- 🏷️ Optional aircraft labels
- 🌍 Configurable radar center coordinates
- 📶 WiFi captive portal setup
- 🔑 Secure API credential storage
- 💾 Settings saved in NVS
- ⚡ Built with ESP-IDF and LVGL

---

## Hardware

- Elecrow CrowPanel Advance 7" ESP32-S3 HMI Display **V1.2**
- 3D printed case

---

## Software

- ESP-IDF
- LVGL 8
- SquareLine Studio
- FreeRTOS
- OpenSky Network REST API

---

## Setup

1. Go to the [Tech Talkies Flasher](https://techtalkies.github.io/flash.html) page.
2. Select the Flight Radar V2
3. Flash and enjoy!

---
## WiFi Configuration

WiFi setup is performed directly on the touchscreen. On the WiFi configuration page,

1. Select your WiFi network.
2. Enter the password using the on-screen keyboard.
3. Save the settings.

The device will automatically connect to the configured network on future boots.

---

## OpenSky API Configuration

An OpenSky Network account is required for live aircraft data. Create one and download the API credentials: https://opensky-network.org/

After connecting the device to WiFi:

1. Note the IP address displayed in the status bar.
2. Open that IP address in a web browser on the same network.
3. Browse the downloaded API credentials file.
4. Upload the credentials.

The credentials are securely stored in NVS and only need to be configured once. Reboot to start the radar.

---

## Radar Settings

The following settings can be configured directly on the touchscreen:

- Latitude
- Longitude

These settings are automatically saved and restored after reboot.

---

## Screens

- Live Radar
- Aircraft Information
- WiFi Setup
- OpenSky API Configuration
- Radar Settings

---

## Roadmap

- [ ] Touch aircraft selection
- [ ] Aircraft trails
- [ ] Aircraft icons by category
- [ ] Day/Night themes
- [ ] Adjustable radar range
- [ ] Airport database
- [ ] Aircraft search
- [ ] Distance rings
- [ ] ADS-B receiver support
- [ ] Offline mode

---

## License

MIT License

---

If you build one, I'd love to see it! Feel free to open an issue or submit a pull request with improvements.
