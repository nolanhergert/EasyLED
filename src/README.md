What features do people care about! Maybe just get a minimum viable product out the door and then start to iterate on what your customers care about :)

Minimum Viable Product
  * DONE! Select from available patterns
  * Save preferences across power cycle and firmware update
     * Setting one value (say pattern) doesn't change other values in pin struct. The whole pin struct is sent in to FastLED updater. Need to remove comment in that updater
     that prevents length from getting updated
     * Maybe can just use eeprom rotate, don't need the full flashee script.
  * Select preferred colors and have them expressed in patterns?
  * Optionally encase in resin for weatherproofing. Maybe have longer usb cable? 
    * Coat pin ends with hot glue until ready to use?
    * Can hold down cables to board with hot glue
    * How to make mold that is reusable
  * Wifi turns off after N (60?) seconds if unused for security? If you have 
  * 4-strip version can have half the resin / board size? Should be much smaller...nice!

Somehow wled.me works on android?

=== "Good" Patterns from WLED ===
  * Biking (alerting, speed, motion): Chase, colorwaves?, Lighthouse, heartbeat (not accurate though), meteor!, noise 2 and 4?, Pride 2015, Running, Scanner / Scanner Dual, 
  * Indoors? Sweep, rain, Twinklefox
  * 


Make it easy to program an external button to do an action? (was thinking of horn)

Should save off previous settings in a backwards compatible and forwards compatible way.
  * Use ESP_EEPROM library too
  * Should have a way to reset EEPROM settings back to normal, externally 
controllable without opening hot glue case?

Allow to set wifi credentials and whether to connect to a home wifi instead.

Would like led selection to be text box with plus / minus?


Website:
  * Am I running out of RAM? If so, load <static> website from PROGMEM (flash) https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html. See how slow this is?  To speed things up, maybe allow caching but need to play with etags to invalidate cache on new page update?

=== Some remaining easy ===
Also try to make the ESP8266 set up so android/others don't complain about not having web access and boot you off the network when on home wifi.
  * It's a setting on Android/iPhone to accept this. Just need to record some setup


=== Notes ===
== Number Pickers ==
https://www.jqueryscript.net/blog/best-number-input-spinner.html#:~:text=Simple%20Number%20Spinner%20Input%20Plugin,the%20'%2B'%20or%20'%2D'%20buttons.

Going to roll my own (simple) one though.
== DNS ==
Works great on everything except Android. Waiting for this issue to get resolved: https://issuetracker.google.com/issues/140786115

I guess I can keep "easyled.local" set up though. Not so bad. For Android it'll stay as an IP address, 192.168.4.1 works fine.
== WLED Patterns ==
Described here: https://github.com/Aircoookie/WLED/wiki/List-of-effects-and-palettes, and copied to FX.h and FX.cpp
== C++ Help ==
Great: https://www.learncpp.com/
== Wifi Management ==
Might want to use this pre-done thing instead: https://github.com/rjwats/esp8266-react
  * Allow to set wifi credentials and whether to connect to a home wifi instead.

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
