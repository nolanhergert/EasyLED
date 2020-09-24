#include <c_types.h>
#include <ErriezCRC32.h>
#include "settings.h"
#include <EEPROM.h>
#include "common.h"


// Move 
/*
void Pin::ParsePinArg(EasyLEDPin *pin, String argName, String argValue) {
  Serial.print("Parsing: ");
  Serial.print(argName);
  Serial.print(", ");
  Serial.println(argValue);
  if (argName == "function") {
    pin->function = argValue.toInt();
    // TODO: Switch on function
  } else if (argName == "num_leds") {
    pin->num_leds = argValue.toInt();
    UpdateLedStrip(pin);
  } else if (argName == "pattern") {
    pin->pattern = argValue.toInt();
    UpdateLedStrip(pin);
  } else if (argName == "color0") {
    // Thanks Michael! https://stackoverflow.com/a/3409211/931280
    /* WARNING: no sanitization or error-checking whatsoever */
    
    int pos = 0;
    for (int count = 0; count < 3; count++) {
      // 1 = Skip the "#" symbol
      sscanf(&(argValue[1+pos]), "%2hhx", &(pin->colors[0].raw[count]));
      pos += 2;
    }
    UpdateLedStrip(pin);
    //pin->colors[0].red = std::stoul(argValue[1], nullptr, 16);
  } else {
    Serial.print("Match not found for: ");
    Serial.println(argName);
  }
}
*/

void Settings::setDefaults() {
  general.version.major = 0;
  general.version.minor = 1;// = {0,1,0,0};
  general.write_count = 0;
}

// IsWriting:
//   FALSE = read
//   TRUE = write
// Reference: https://arduino-esp8266.readthedocs.io/en/latest/libraries.html#eeprom
// Also restore defaults if reading and crc doesn't match
bool Settings::IO(bool IsWriting) {
  bool rc = RC_FAILURE;
  uint16 i = 0;
  uint32 crc, crcCopy = 0;

  Serial.print("Settings::IO called with ");
  Serial.println(IsWriting);

  EEPROM.begin(FLASH_PAGE_SIZE);

  // Read in initial copy if needed
  for (i = 0; i < sizeof(*this); i++) {
    if (!IsWriting) {
      ((uint8 *)this)[i] = EEPROM.read(i);
    }
  }

  Serial.print("Version: ");
  Serial.print(general.version.major);
  Serial.print(".");
  Serial.println(general.version.minor);
  Serial.print("Write count: ");
  Serial.println(general.write_count);
  

  if (IsWriting) {
    general.write_count++;
  }

  // Check CRC
  // Make backup copy of existing crc
  crcCopy = general.crc;
  general.crc = 0;

  crc = crc32Buffer(this, sizeof(*this));
  if (!IsWriting && crc != crcCopy) {
    // Reading from EEPROM and crc doesn't check out, restore defaults and exit
    Serial.print("CRC didn't align. Restoring defaults!\n");
    setDefaults();
    rc = RC_SUCCESS;
    goto Finish;
  } else {
    // Either the crc matched or we want to use the new crc value for writing
    general.crc = crc;
  }

  if (IsWriting) {
    for (i = 0; i < sizeof(*this); i++) {
      // Write to internal copy of eeprom
      EEPROM.write(i, ((uint8 *)this)[i]);
    }

    // Commit to flash
    if (!EEPROM.commit()) {
      Serial.print("ERROR! EEPROM commit failed\n");
      goto Finish;
    }

    // Read data back from flash to ensure integrity
    for (i = 0; i < sizeof(*this); i++) {
      if (EEPROM.read(i) != ((uint8 *)this)[i]) {
        Serial.print("ERROR! EEPROM failed verification\n");
        goto Finish;
      }
    }
  }

  rc = RC_SUCCESS;

Finish:
  // Release cached copy of eeprom from heap
  EEPROM.end();
  return rc;
}