# WiiNunchuck_Mouse

This repository contains a few examples on how to use the Wii Nunchuck Pro Micro Adapter with the BlueMicro840 to make the Nunchuck into a mouse. 

![alt text](https://github.com/jpconstantineau/WiiNunchuck_Mouse/raw/main/img/Nunchuck_BLE.jpg "Nunchuck_BLE on Windows 10")

| Example | Controller | Connection | Required Libraries |
| --------| ---------- | ---------- | ------------------ |
| BlueMicro840_USB_Mouse | BlueMicro840 | USB | Adafruit_TinyUSB, NintendoExtensionCtrl |
| BlueMicro840_BLE_Mouse | BlueMicro840 | BLE | bluefruit, NintendoExtensionCtrl |
| BlueMicro840_HID_Mouse | BlueMicro840 | USB and BLE (Automatic Selection) | bluefruit, Adafruit_TinyUSB, NintendoExtensionCtrl |

Note: The example code has not been optimized to minimize power consumption when running on battery power.  As such, it is not known how much current the Nunchuck uses.  We also don't know how long a battery will last.

<a href="https://www.tindie.com/stores/jpconstantineau/?ref=offsite_badges&utm_source=sellers_jpconstantineau&utm_medium=badges&utm_campaign=badge_large"><img src="https://d2ss6ovg47m0r5.cloudfront.net/badges/tindie-larges.png" alt="I sell on Tindie" width="200" height="104"></a>
