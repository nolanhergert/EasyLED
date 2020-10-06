#include <c_types.h>
#include <ErriezCRC32.h>
#include "settings.h"
#include <EEPROM.h>
#include "common.h"

// #001122 -> CRGB
CRGB HTMLColorCodeToCRGB(const String& s) {
  // Thanks Michael! https://stackoverflow.com/a/3409211/931280
  // WARNING: no sanitization or error-checking whatsoever
  CRGB c;
  sscanf(s.c_str(), "#%2hhx%2hhx%2hhx", &c.raw[0], &c.raw[1], &c.raw[2]);
  return c;
}

void CRGBToHtmlColorCode(CRGB c, String& s) {
  // Thanks Doug! https://stackoverflow.com/a/2342176/931280
  char temp[8];
  snprintf(temp, sizeof(temp), "#%02x%02x%02x", c.raw[0], c.raw[1], c.raw[2]);
  s = temp;
}


// Very stack inefficient probably? Not sure...
// Probably want to do something like this: https://github.com/bblanchon/ArduinoJson/issues/166

// Don't need to be future compatible, since we're also serving 
// that the user is interacting with.... is that logic right?
// So we can use a StaticJsonDocument on the stack
// Stack size is 4K, so need to use heap
//DynamicJsonDocument<SettingsJsonCapacity> doc;
void Settings::serialize(String& s) {
  /*
  StaticJsonDocument<500> jsonBuffer;

  JsonObject &root = jsonBuffer.createObject();
  JsonArray &tempValues = root.createNestedArray("temperature");
  tempValues.add(5);

  String json;
  serializeJsonPretty(root, json);
  return json;
  */
  int i, j = 0;
  DynamicJsonDocument doc(SettingsJsonCapacity);


  // General settings
  String ver;
  general.version.toString(general.version, ver);
  doc["version"] = ver;
  doc["writeCount"] = general.writeCount;
  doc["crc"] = general.crc;
  doc["numPins"] = general.numPins;
  
  JsonArray pinsJson = doc.createNestedArray("pins");
  // Settings for each pin
  for (i = 0; i < general.numPins; i++) {
    JsonObject pin = pinsJson.createNestedObject();
    pin["index"] = pins[i].index;
    pin["function"] = pins[i].function;
    pin["pattern"] = pins[i].pattern;
    pin["num_leds"] = pins[i].num_leds;
    pin["offset"] = pins[i].offset;
    JsonArray colors = pin.createNestedArray("colors");
    String HTMLColorCode;
    for (j = 0; j < NUM_COLORS; j++) {
      CRGBToHtmlColorCode(pins[i].colors[j], HTMLColorCode);
      colors.add(HTMLColorCode);
    }
  }

  serializeJsonPretty(doc, s);
  // doc is freed once it goes out of scope
}

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
    / WARNING: no sanitization or error-checking whatsoever
    
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
  // Zero out Settings struct
  //this = {};
  memset(this, 0, sizeof(*this));

  general.version.major = 0;
  general.version.minor = 1;// = {0,1,0,0};
  general.writeCount = 0;
  general.numPins = NUM_PINS;
  
  for (int i = 0; i < general.numPins; i++) {
    pins[i].index = i;
    pins[i].colors[0] = CRGB::HTMLColorCode::Blue;
  }
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
  Serial.println(general.writeCount);
  

  if (IsWriting) {
    general.writeCount++;
  }

  // Check CRC
  // Make backup copy of existing crc
  crcCopy = general.crc;
  general.crc = 0;

  crc = crc32Buffer(this, sizeof(*this));
  if (!IsWriting && crc != crcCopy) {
    // Reading from EEPROM and crc doesn't check out, create defaults and
    // write to flash
    Serial.print("CRC didn't align. Creating defaults!\n");
    setDefaults(); // zeroes entire struct, including crc
    general.crc = crc32Buffer(this, sizeof(*this));
    IsWriting = true;
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