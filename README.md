# LedStripController
Main use cases:
  * Control multiple independent led strips from one controller without soldering!
  * Portable outdoor control and mounting (optional)
  * Very little setup required!

## PCBs
  * https://oshwlab.com/nhergert/easyled
  * https://oshwlab.com/nhergert/easyledv2_usbexpander

# Instructions

# Behavior
  * Wifi is set to turn off after 120 seconds if not accessed on startup. If the wifi is accessed, it will continue to stay on until power is removed.
  * Click "Save Changes" to persist the settings to the microcontroller permanent storage (flash).

Works best with Platform.io.

## Requirements
  * Install FastLED, ErriezCRC32, ArduinoJSON library (done automatically in platform.io)
  * Run autogen.py (done automatically in platform.io)
