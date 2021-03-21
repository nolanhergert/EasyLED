r'''
Detect *in parallel* when:
  * A new EasyLED module is detected on an available USB to Serial adapter
  * Upload firmware
  * Confirm that access point exists (don't need to connect to it, I think)
    * Plus it allows you to run things in parallel if you don't connect.
  
## Hardware requirements
Only need a cheap UART to USB adapter plus some cheap components:
https://www.instructables.com/FTDI-ESP8266-Definitive-Wiring/

## Detect power on of new unflashed ESP8266
Just let esptool.py run continually

## Upload firmware
Current command: (just run Platform.io verbose upload)
"c:\users\nolan-thinkpad\.platformio\penv\scripts\python.exe" "C:\Users\Nolan-Thinkpad\.platformio\packages\tool-esptoolpy\esptool.py" --before no_reset --after soft_reset --chip esp8266 --port "COM8" --baud 1500000 write_flash 0x0 .pio\build\d1_mini\firmware.bin

## Read serial port
Doesn't really work for some reason? More details on the hardware requirements
but I don't need it yet.

## Confirm access point exists
Can read MAC address from esptool output, so can figure out the
wifi access point name.

Using python, need to use subprocess to call the terminal. Check
for existence of known name, and done I think!

Might want to confirm can view web page and send something? Not sure.


'''


import subprocess

# Thanks https://stackoverflow.com/a/60405404!
# if windows:
r = subprocess.run(["netsh", "wlan", "show", "network"], capture_output=True, text=True).stdout
ls = r.split("\n")
ssids = [v.strip() for k,v in (p.split(':') for p in ls if 'SSID' in p)]
print(ssids)
# If Linux: (replace wlp1s0 with the interface name from ifconfig)
# sudo iw dev wlp1s0 scan | grep SSID