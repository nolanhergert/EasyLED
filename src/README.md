What features do people care about! Maybe just get a minimum viable product out the door and then start to iterate on what your customers care about :)

Should save off previous settings in a backwards compatible and forwards compatible way.
  * Use ESP_EEPROM library too

Website:
  * Should some kind of auto-generation if needed for the build process to save copy/pasting work. However, should be able to do the same process/page for each strip.
  * Am I running out of RAM? If so, load <static> website from PROGMEM (flash) https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html. See how slow this is?  To speed things up, maybe allow caching but need to play with etags to invalidate cache on new page update?
  
Also try to make the ESP8266 set up so android/others don't complain about not having web access and boot you off the network when on home wifi.
  * It's a setting on Android/iPhone to accept this

Allow to set wifi credentials and whether to connect to a home wifi instead.

Should have a way to reset EEPROM settings back to normal, externally controllable without opening hot glue case?

Popup screen should have:
  * Number of LEDs on strip, or if it has something else on it (Servo, motor, horn, single PWM led)
  * Pattern
  * Speed
  * Set colors (primary colors?)
  * Paste script? (can be dynamic)

Can select from list of patterns to make a custom pattern list? (sequence)

Pattern offset from another strip? Duplicated?

Pattern programming. Transition time + list of patterns.
