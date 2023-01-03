This is a Clone Hero guitar on an ESP32

This was based on ESP32-BLE-Gamepad library's MultipleButtonsDebounce example

Depends on Bounce2 and ESP32-BLE-Gamepad libraries

https://github.com/lemmingDev/ESP32-BLE-Gamepad

https://github.com/thomasfredericks/Bounce2

ESP32_Guitar.ino is an Arduino IDE project file and requires 2 packages from the Library Manager:
  -Bounce2
  -ESP32-BLE-Gamepad
  
Arduino IDE also requires some setup to make it work with the ESP32.
   
GPIO_pins.txt contains information on what GPIO pins and button IDs were used.
