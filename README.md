# esp8266-Matrix-clock
This project is a smart WiFi-capable clock based on an ESP8266.

-time via NTP server 
-Current weather via Wonderground API

The 3D part is from the AWTRIX project:
https://www.thingiverse.com/thing:2791276

The Led Matrix is made by using pannel of 8\*8 WS2812 led. The code can be adapt to different Tile configuration: **1\*4** is the current use in the clock. But you can do **2\*4** or **5*5**.

TODO:
-change the code to drive more than 256 LEDs
-add luminosity settings
-add PIR sensor for sleep mode while nobody's here
-add support for bitmap image (logo, image, animation,....)
-clean code
