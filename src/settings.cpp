#include "main.h"
#include <c_types.h>


// We have 4096 bytes in one flash page
class Settings {







  void init() {
    EEPROM.begin(sizeof(Settings));  

  }
}

void setup_settings() {
  
}


bool WriteToEEPROM() {
  bool success = false;
  int i = 0;
  // Write to EEPROM memory
  for (i = 0; i < sizeof(Cache); i++) {
    EEPROM.write(i, ((uint8 *)&cache)[i]);
  }

  // Persist to flash
  success = EEPROM.commit();
  if (!success) {
    goto Finish;
  }

  // Confirm bytes written out
  for (i = 0; i < sizeof(Cache); i++)

Finish:
  if ()
}