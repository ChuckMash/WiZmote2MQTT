# WiZmote2MQTT

wizmote2mqtt runs on ESP8266 or ESP32
---
Works with out-of-the-box WiZmotes with no modification required
---
* Relays WiZmote button presses to MQTT
* Can use multiple WiZmotes simultaneously
* Easy to use with Home Assistant
* Uses EspMQTTClient for high reliability
* Activity Indicator LED
* Also sends JSON messages over Serial when WiZmote button pressed
* Tested on
  * ESP32 D1 Mini
  * ESP8266 D1 Mini


---

![WiZmote](https://i.imgur.com/sWICujZ.jpg)

---

### Topics

Device Status (online/offline): wizmote/wizmote2mqtt-xxxxxxxxxxxx (ESP MAC Address)

Publishes received WiZmote Button codes to wizmote/wizmote-xxxxxxxxxxxx (WiZmote BSSID)
| Button | Button Code|
|-------|------------|
| ON    | 1          |
| OFF   | 2          |
| 🌙    | 3          |
| 1     | 16         |
| 2     | 17         |
| 3     | 18         |
| 4     | 19         |
| -     | 8          |
| +     | 9          |

---


