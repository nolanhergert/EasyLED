#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#include <c_types.h>
#include <FastLED.h>
#include <inttypes.h>
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
#define NUM_PINS 8

// Do not modify this without also providing backwards compatibility with
// earlier software revisions (with different sizes)!
// Overprovisioning so we have plenty of runway in the future while
// maintaining backwards compatibility
#define SETTINGS_GENERAL_SIZE 1024
#define EASY_LED_PIN_SIZE 256
_Static_assert(SETTINGS_GENERAL_SIZE + EASY_LED_PIN_SIZE * NUM_PINS <= FLASH_PAGE_SIZE, "");

#include <inttypes.h>

#include <cstdio>
#include <string>
#include <ESP8266WebServer.h>




// Thanks sourcey!
// https://sourcey.com/articles/comparing-version-strings-in-cpp
struct Version
{
  uint16 major = 0, minor = 0, revision = 0, build = 0;

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
    char temp[3+3+3+4+1+1]; // 00.00.00.0000, not sure why extra one is needed
    snprintf(temp, sizeof(temp), "%02d.%02d.%02d.%04d", major, minor, revision, build);
    s = temp;
  }
  void fromString(const String& s) {
    sscanf(s.c_str(), "%02" SCNd16 ".%02" SCNd16 ".%02" SCNd16 ".%04" SCNd16, &major, &minor, &revision, &build);
  }
};

#define SETTINGS_GENERAL_BYTE_COUNT 18
typedef struct  {
  Version version;
  uint16 writeCount;
  uint32 crc; // Calculated over entire flash page. Assumed to be 0 during calculation
  uint8 numPins; // Always store NUM_PINS in flash, but limit display on GUI to numPins
  uint8 brightness; // 0-255
  // Wifi ID, password, and encryption level?

  // When adding fields:
  //   * Increase byte count (SETTINGS_GENERAL_BYTE_COUNT)
  //   * Increase number of fields in JsonDocument capacity below
  //   * Add fields to serialize() and deserialize()
  uint8 reserved[SETTINGS_GENERAL_SIZE-SETTINGS_GENERAL_BYTE_COUNT];
} SettingsGeneral;
_Static_assert(SETTINGS_GENERAL_SIZE == sizeof(SettingsGeneral), "");
const int SettingsGeneralJsonCapacity = JSON_OBJECT_SIZE(5);

#define NUM_COLORS 5
#define EASY_LED_PIN_BYTE_COUNT 24
struct EasyLEDPin {
  uint8 index;
  uint8 function; // >0
  uint16 pattern; // >0
  uint16 num_leds;// >0
  sint16 offset;  
  // In decreasing importance
  CRGB colors[NUM_COLORS]; // rgb
  // When adding fields:
  //   * Increase byte count (EASY_LED_PIN_BYTE_COUNT)
  //   * Increase number of fields in JsonDocument capacity below
  //   * Add fields to serialize() and deserialize()
  uint8 reserved[EASY_LED_PIN_SIZE - EASY_LED_PIN_BYTE_COUNT];
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
    EasyLEDPin pins[NUM_PINS];
    // There's some room yet
    uint8 reserved[FLASH_PAGE_SIZE - sizeof(general) - sizeof(pins)];

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
    //void deserialize(const String& json);
    // Simple and fast solution for now
    bool ParseURLArgs(ESP8266WebServer& server);
    

    private:
      // IsWriting:
      //   FALSE = read
      //   TRUE = write
      // Reference: https://arduino-esp8266.readthedocs.io/en/latest/libraries.html#eeprom
      bool IO(bool IsWriting);

      void setDefaults();
};
// char (*__kaboom)[sizeof(Settings)] = 1;
// Make sure size is appropriate
_Static_assert(sizeof(Settings) == FLASH_PAGE_SIZE, "");
// Created using https://arduinojson.org/v6/assistant/
const int SettingsJsonCapacity = SettingsGeneralJsonCapacity + 
                                 // pins array
                                 JSON_OBJECT_SIZE(1) + 
                                 // NUM_PINS pins
                                 JSON_ARRAY_SIZE(NUM_PINS) + NUM_PINS*EasyLEDPinJsonCapacity +
                                 // magical number, not sure where it comes from
                                 JSON_ARRAY_SIZE(2);


#endif // _SETTINGS_H_