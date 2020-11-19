#include <c_types.h>
#include <ErriezCRC32.h>
#include "settings.h"
#include <EEPROM.h>
#include "common.h"


#define DEFAULT_BRIGHTNESS (uint8)255 // can be dynamically changed though
#define DEFAULT_MAX_MILLIWATTS 4 * 1000 // 4V * 1000mA, need to play with this some more

// "RRGGBB" -> 0xRRGGBB
CRGB HTMLColorCodeToCRGB(const String& s) {
  // Thanks Michael! https://stackoverflow.com/a/3409211/931280
  // WARNING: no sanitization or error-checking whatsoever
  CRGB c;
  sscanf(s.c_str(), "%2hhx%2hhx%2hhx", &c.raw[0], &c.raw[1], &c.raw[2]);
  return c;
}

// 0xRRGGBB -> "RRGGBB"
void CRGBToHtmlColorCode(CRGB c, String& s) {
  // Thanks Doug! https://stackoverflow.com/a/2342176/931280
  char temp[8];
  snprintf(temp, sizeof(temp), "%02x%02x%02x", c.raw[0], c.raw[1], c.raw[2]);
  s = temp;
}

// Very stack inefficient probably? Not sure...
// Probably want to do something like this: https://github.com/bblanchon/ArduinoJson/issues/166

// Don't need to be future compatible, since we're also serving 
// that the user is interacting with.... is that logic right?
// Stack size is 4K, so probably should use heap
void Settings::serialize(String& s) {
  int i, j = 0;
  DynamicJsonDocument doc(SettingsJsonCapacity);
  
  // General settings
  String version;

  general.version.toString(version);
  doc["version"] = version;
  doc["writeCount"] = general.writeCount;
  doc["crc"] = general.crc;
  doc["numPins"] = general.numPins;
  doc["brightness"] = general.brightness;
  doc["maxPowerMilliwatts"] = general.maxPowerMilliwatts;
  
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

/*
void Settings::deserialize(const String &json) {
  DynamicJsonDocument doc(SettingsJsonCapacity);
  deserializeJson(doc, json);
  int i, j = 0;  // Settings for each pin

  
  // Save a few clock cycles this way https://arduinojson.org/v6/api/jsonobject/containskey/
  uint8 function; // >0
  uint16 pattern; // >0
  uint16 num_leds;// >0

  // Only deserialize what we know about. Ignore? everything else
  // Should be able to handle partial updates too...
  
  

  // Don't need or want to modify these normally
  //doc["version"] = ver;
  //doc["writeCount"] = general.writeCount;
  //doc["crc"] = general.crc;
  //doc["numPins"] = general.numPins;

  // Going a little weird here because 0 is a valid brightness value
  // https://arduinojson.org/v6/api/jsonobject/containskey/
  JsonVariant brightness = doc["brightness"];
  if (!brightness.isNull()) {
    general.brightness = brightness.as<uint8>();
  }

  JsonArray pinsJson = doc["pins"];
  if (!pinsJson) return;


  for (i = 0; i < general.numPins; i++) {
    JsonObject pin = pinsJson[i];
    if (!pin) continue; // no pin value provided

    // Don't want to change this value
    //pin["index"] = pins[i].index;
    function = pin["function"];
    if (function > 0) {
      pins[i].function = function;
    }
    pattern = pin["pattern"];
    if (pattern > 0) {
      pins[i].pattern = pattern;
    }
    num_leds = pin["num_leds"];
    if (num_leds > 0) {
      pins[i].num_leds = num_leds;
    }
    JsonVariant offset = pin["offset"];
    if (!offset.isNull()) {
      pins[i].offset = offset.as<sint16>();
    }
    JsonArray colors = pin.createNestedArray("colors");
    if (!colors) {
      String HTMLColorCode;
      for (j = 0; j < NUM_COLORS; j++) {
        CRGBToHtmlColorCode(pins[i].colors[j], HTMLColorCode);
        colors.add(HTMLColorCode);
      }
    }
  } 
}
*/



void Settings::setDefaults() {
  // Zero out Settings struct
  //this = {};
  // Save off a few fields that shouldn't be changed before zeroing
  uint16 numPins = general.numPins;
  uint16 writeCount = general.writeCount;


  memset(this, 0, sizeof(*this));

  general.version.major = 0;
  general.version.minor = 1;// = {0,1,0,0};
  general.writeCount = writeCount;
  general.numPins = numPins;
  general.brightness = DEFAULT_BRIGHTNESS;
  general.maxPowerMilliwatts = DEFAULT_MAX_MILLIWATTS;
  
  for (int i = 0; i < general.numPins; i++) {
    pins[i].index = i;
    pins[i].colors[0] = CRGB::HTMLColorCode::Blue;
    pins[i].pattern = i;
    pins[i].num_leds = 10;
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
  if (!IsWriting) {
    for (i = 0; i < sizeof(*this); i++) {
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
      Serial.println("ERROR! EEPROM commit failed");
      goto Finish;
    }

    // Read data back from flash to ensure integrity
    for (i = 0; i < sizeof(*this); i++) {
      if (EEPROM.read(i) != ((uint8 *)this)[i]) {
        Serial.println("ERROR! EEPROM failed verification");
        goto Finish;
      }
    }
  }
  Serial.println("Success in writing to eeprom! Inside function");
  rc = RC_SUCCESS;

Finish:
  // Release cached copy of eeprom from heap
  EEPROM.end();
  return rc;
}