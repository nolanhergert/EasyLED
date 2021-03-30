#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <Arduino.h>
#include <FastLED.h>
#include <ArduinoJson.h>

////////////////////////////////////////////////////////////////
// Helpful compile time print if the below assertions fail
// https://stackoverflow.com/a/35261673/931280
//char (*__kaboom)[offsetof(Settings, pins)] = 1;
////////////////////////////////////////////////////////////////

// Sticking with wearing out one page for simplicity
#define FLASH_PAGE_SIZE 4096
// Conservative estimate of cheap flash modules
#define FLASH_PAGE_MAX_WRITE_COUNT 3000
// Index is 0-7, but pin number on the board and GUI is 1-8
// Be careful of changing this value! Used in a number of places...
// TODO: Is it ok to modify this for a resin-cast 4-pin board? Need to reset to defaults correctly...
#define MAX_PINS 8

// Do not modify this without also providing backwards compatibility with
// earlier software revisions (with different sizes)!
// Overprovisioning so we have plenty of runway in the future while
// maintaining backwards compatibility
#define SETTINGS_GENERAL_SIZE 1024
#define EASY_LED_PIN_SIZE 256
_Static_assert(SETTINGS_GENERAL_SIZE + EASY_LED_PIN_SIZE * MAX_PINS <= FLASH_PAGE_SIZE, "");

#include <inttypes.h>
#include <cstdio>
#include <string>
#include <ESP8266WebServer.h>




// Thanks sourcey!
// https://sourcey.com/articles/comparing-version-strings-in-cpp
struct Version
{
  uint16_t major = 0, minor = 0, revision = 0, build = 0;

	bool operator < (const Version& other)
	{
		if (major < other.major)
			return true;
		if (minor < other.minor)
			return true;
		if (revision < other.revision)
			return true;
		if (build < other.build)
			return true;
		return false;
	}

	bool operator == (const Version& other)
	{
		return major == other.major
			&& minor == other.minor
			&& revision == other.revision
			&& build == other.build;
	}
  void toString(String& s) {
    // Probably not the proper way to do this, but we're on an Arduino
    char temp[3+3+3+4+1]; // 00.00.00.0000
    snprintf(temp, sizeof(temp), "%02d.%02d.%02d.%04d", major, minor, revision, build);
    s = temp;
  }
  void fromString(const String& s) {
    sscanf(s.c_str(), "%02" SCNd16 ".%02" SCNd16 ".%02" SCNd16 ".%04" SCNd16, &major, &minor, &revision, &build);
  }
};


//typedef uint8_t Configuration;
//#define CONFIG_EASYLED_4_PIN  0
//#define CONFIG_EASYLED_8_PIN  1
// Can only use values from this enum
// And it enforces the max size too!
// https://stackoverflow.com/a/1122109/931280
enum class Configuration : uint8_t {
  CONFIG_EASYLED_4_PIN = 0,
  CONFIG_EASYLED_8_PIN = 1,
  // Max size of uint8
};

#define SETTINGS_GENERAL_BYTE_COUNT 24
typedef struct {
  Version version;
  uint16_t writeCount;
  uint32_t crc; // Calculated over entire flash page. Assumed to be 0 during calculation
  Configuration config; // (hardware configuration) 0 = 4-pin EasyLED board, 1 = 8-pin EasyLED board
  uint8_t PADDING_USE_ME_0[1]; // ESP8266 doesn't like "load or store to unaligned address", so we can't pack
  uint8_t numPins; // Always store MAX_PINS in flash, but limit display on GUI to numPins
  uint8_t numPinStructsInSettings; // number of pin structs that follow in settings
  uint8_t PADDING_USE_ME_1[2];
  uint32_t maxPowerMilliwatts;
  // Wifi ID, password, and encryption level?

  // When adding fields:
  //   * Only *append* fields, don't delete or insert
  //   * Try to use padding bytes first. Add padding bytes for unaligned reads/writes
  //   * Increase byte count (SETTINGS_GENERAL_BYTE_COUNT)
  //   * Increase number of fields in JsonDocument capacity below
  //   * Add fields to serialize() and ParsePinVariable() in main.cpp
  uint8_t reserved[SETTINGS_GENERAL_SIZE-SETTINGS_GENERAL_BYTE_COUNT];
} SettingsGeneral;
_Static_assert(SETTINGS_GENERAL_SIZE == sizeof(SettingsGeneral), "");
const int SettingsGeneralJsonCapacity = JSON_OBJECT_SIZE(6);

enum function {
  FUNCTION_ANIMATION = 0x0,
  FUNCTION_PIN_NOT_CONNECTED = 0xFF
};

#define NUM_COLORS 5
#define EASY_LED_PIN_BYTE_COUNT 23
struct EasyLEDPin {
  uint8_t index;
  uint8_t function; // >0
  uint16_t pattern; // >0
  uint16_t num_leds;// >0
  int16_t offset;  // "pattern" offset relative to the first led of a pattern NOT IMPLEMENTED YET
  // In decreasing importance
  CRGB colors[NUM_COLORS]; // rgb
  // When adding fields:
  //   * Only *append* fields, don't delete or insert
  //   * Try to use padding bytes first. Add padding bytes for unaligned reads/writes
  //   * Increase byte count (EASY_LED_PIN_BYTE_COUNT)
  //   * Increase number of fields in JsonDocument capacity below
  //   * Add fields to serialize() and deserialize()
  uint8_t reserved[EASY_LED_PIN_SIZE - EASY_LED_PIN_BYTE_COUNT];
};
_Static_assert(sizeof(EasyLEDPin) == EASY_LED_PIN_SIZE, "");
// This doesn't technically need to be in the .h file since we aren't exposing it
// to other files, but it makes adding fields to EasyLEDPin easier
//                                                  Colors                      rest of fields
const int EasyLEDPinJsonCapacity = JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(5);




#define SETTINGS_IO_READ false
#define SETTINGS_IO_WRITE true

// This should be a Singleton class (only one copy), but not implementing it yet
// It's C++, so structs are classes too!
struct Settings {
    SettingsGeneral general;
    EasyLEDPin pins[MAX_PINS];
    // There's some room yet
    uint8_t reserved[FLASH_PAGE_SIZE - sizeof(general) - sizeof(pins)];

    // Default constructor
    Settings() {};

    void read() {
      // Automatically read from eeprom on init
      IO(SETTINGS_IO_READ);
    }

    bool save() {
      return IO(SETTINGS_IO_WRITE);
    }

    // String pointer as output so it doesn't go out of scope
    void serialize(String& json);
    // String as input and *modified* in the function
    // Untested!
    // Going with fast and simple solution for now,
    // ParseURLArgs() in main.cpp
    //void deserialize(const String& json);
    void setDefaults();

    private:
      // IsWriting:
      //   FALSE = read
      //   TRUE = write
      // Reference: https://arduino-esp8266.readthedocs.io/en/latest/libraries.html#eeprom
      bool IO(bool IsWriting);


};
// Make sure size is appropriate
_Static_assert(sizeof(Settings) == FLASH_PAGE_SIZE, "");
// Created using https://arduinojson.org/v6/assistant/
const int SettingsJsonCapacity = SettingsGeneralJsonCapacity +
                                 // pins array
                                 JSON_OBJECT_SIZE(1) +
                                 // MAX_PINS pins
                                 JSON_ARRAY_SIZE(MAX_PINS) + MAX_PINS*EasyLEDPinJsonCapacity +
                                 // magic number, not sure where it comes from
                                 JSON_ARRAY_SIZE(2);


#endif // _SETTINGS_H_