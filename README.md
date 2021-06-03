# Arduino-smart-home-switch
A way to add 4 (of 9) buttons to Path/Json request,
For example it can be used to send commands to [MilightHub](https://github.com/sidoh/esp8266_milight_hub) or [Domoticz](https://www.domoticz.com/). This is useful to turn off/on smart lights and such.

This repo replaces the deprecated [Arduino-MiLightHub_switch](https://github.com/jellewie/Arduino-MiLightHub_switch) project, but is NOT compatible with it

# What you need
- An hub (and its IP ofc) to send data to (for example an ESP8266 with [esp8266_milight_hub](https://github.com/sidoh/esp8266_milight_hub))
- an ESP32 board
- [an PCB and all it parts](https://easyeda.com/jellewietsma/smart-home-switch)

# How it works
1. Make sure the hub is already set-up and working, and you know it's IP.
2. Make sure you have an ESP with the PCB, you can also design your own yourself but mine is linked in this project (make sure to read the schematics, they should explain themself)
3. You can either setup the settings like SSID and passwords and such in the code, but you can also just upload the sketch and power it on and set it up: It will go into APMODE (since it can’t connect to WIFI) connect to it and go to it's IP (192.168.4.1)
4. You will get a window with 'SSID Wi-Fi name', 'Wi-Fi password', 'the IP of the Hub like "192.168.4.1"', 'hup port, use default 80 when unsure', and such. Change or fill these in and submit. see 'Examples' for some cases that I've used them for. The ESP will save these settings and reboot (Leaving fields blank will skip updating them, changing a field to be spaces only will clear it)

#Examples 
Example config for [MilightHub](https://github.com/sidoh/esp8266_milight_hub)
For more see https://sidoh.github.io/esp8266_milight_hub/branches/1.10.6/ Please note not all commands might be supported by your bulb
Soft Setting | What command the button sends                      | Note
------------ | -------------------------------------------------- | -------------
Path_A1      | /gateways/0xF001/rgb_cct/1                         | Device Id="0xF001", remote Type="rgb_cct", Group = "1"
Json_A1      | {'commands':['toggle']}                            | Example of toggle
Json_A2      | {'brightness':60,'color':'255,50,10','state':'On'} | Example of RGB
Json_A3      | {'brightness':255,'color_temp':999,'state':'On'}   | Example of CC/WW
Json_A4      | {'brightness':255,'color_temp':1,'state':'On'}     | Another example of CC/WW
  
Example config for [Domoticz](https://www.domoticz.com/)
Soft Setting | Input                                                   | Note
------------ | ------------------------------------------------------- | -------------
Path_A1      | /json.htm?type=command&param=switchlight&idx=999&switchcmd=Toggle&level=0&passcode | (Example of toggle) idx=999
Path_A2      | /json.htm?type=command&param=setcolbrightnessvalue&idx=999&color={"m":3,"t":0,"r":255,"g":254,"b":253,"cw":0,"ww":0}&brightness=100 | (Example of RGB) idx=999, r:255, g:254, b:253, brightness=100

# LED feedback
There a lot of blinking patterns, but I tried to list them all here
- (Main LED) Turns on on boot, and turn of when done
- (Main LED) Turns on when a long press, and turn of when released
- (Main LED) will blink every 100ms to show we are not connected (APMODE)
- (Main LED) will blink every 500ms to show we are trying to connect
- (Button LED) Turns on when the button is pressed, and only off after it's done (or a connection timeout)
- (Button LED) Blinks every 50ms on boot if pressed (should be released on boot)
- (Button LED) Blinks every 10ms if we are less then 1000ms from restarting (super long press)
- (Button LED) Blinks every 1000ms if the server (and Over The Air update) is enabled

# Button actions
- (Button short press) execute action; send the json to "Hub_IP : Hub_Port Path" where path incluses prefix '/'
It stops after 5 failed tries, or 2 failed connections, or 1 if send Json is wrong
- (Button long press) Hold any button for 3s and enable OTA+Web interface (Time_StartLongPressMS=3000ms)
- (Button extreme long press) Hold any button for 15s to reboot the ESP (Time_ESPrestartMS=15000ms)

# Testing urls
There is a tes URL path where the commands can be tested. These would execute the DoRequest command and trigger the LED's just as the buttons would do.
URL                                                               | The action
----------------------------------------------------------------- | ------------
[smart-switch.local/test](http://smart-switch.local/test)         | Run test A1
[smart-switch.local/test?a=2](http://smart-switch.local/test?a=2) | Run test A2
[smart-switch.local/test?B=3](http://smart-switch.local/test?B=3) | Run test B3
[smart-switch.local/restart](http://smart-switch.local/restart)   | Restart the ESP
http://192.168.1.5/restart
  
# Soft settings
There are multiple soft settings, these are settings that can be changed after the sketch has been uploaded, but are still saved so they can be restored after a reboot.
The most up-to-date values can be found in the top of the [WiFiManagerBefore.h](Arduino/WiFiManagerBefore.h) source file, and can only be set in [smart-switch.local/ip](http://smart-switch.local/ip).
Note that the character " and TAB (EEPROM_Seperator) cannot be used, these will be replaced with ' and SPACE respectively. Leave black to skip updating these, use spaces ' ' to clear the values
- **Hub IP** The ip to connect to when a button is pressed
- **Hub port** The port to connect to on the given IP
- **Rotation [A/B]** The rotation of the button, can be "UNUSED", "NORMAL" (Arrow up), "RIGHT" (Arrow right), "UPSIDE_DOWN" (Arrow down), or "LEFT" (Arrow left)
- **Path A/B 1/2/3/4** The path to connect to, includes a '/' prefix, when none is given the path of Path_A/B_1 will be used
- **Json A/B 1/2/3/4** The json data to send (if any)

# OTA (Over The Air update)
This page can be accessed on [smart-switch.local/ota](http://smart-switch.local/ota) (or 'IP/ota') and enables you to update firmware over WiFi.
On this page is a 'choose file' button where the new version can be selected. Make sure the right, compatible, most updated file is selected ("Smart_clock.bin"). This can be downloaded from [releases](https://github.com/jellewie/Arduino-smart-home-switch/releases). 
After which the 'Upload' button needs to be press for the update process to begin, the unit will automatically update and reboot afterwards.
Note that [SoftSettings](#soft-settings) are preserved.

# Appendix
* Firmware
[This is included in this repository](Arduino)
* PCB & schematic
[This is included in this repository](Schematic-PCB)
Beta/ updated version on [EasyEDA](https://easyeda.com/jellewietsma/smart-home-switch)
* 3D models
[These are included in this repository](3DModel)
