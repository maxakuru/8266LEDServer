﻿# 8266LEDServer

## Prepare
1. Arduino IDE
2. Install ESP8266 for Arduino with [Boards Manager](https://github.com/esp8266/Arduino#installing-with-boards-manager)
3. Get [DotStar libraries](https://github.com/adafruit/Adafruit_DotStar)

## Configure
1. Set `NUMPIXELS` to number of LEDs you have in your strip
2. Set `ssid` and `password` to your network credentials
3. Optionally set `tableID` to something else
4. Wire up your LEDs and ESP8266
5. Flash controller with `LEDserver` sketch

## Lighting
|Method|Params|Result|
|------|------|------|
|`/POST`|Body `{data: string}` where `string` is hex color codes separated by a delimiter|Splits up `string` based on your set `DELIMITER`, lights up individual LEDs with each color in split. For example, `{data: ff0000;00ff00;0000ff}` would light 3 LEDs with red, green, and blue.
|`/GET`|N/A|Returns the set `tableID` in plaintext, like `TableID: ${tableID}`|
