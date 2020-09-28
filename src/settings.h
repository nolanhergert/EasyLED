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


#include <cstdio>
#include <string>
#include <iostream>

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

	friend std::ostream& operator << (std::ostream& stream, const Version& ver)
	{
		stream << ver.major;
		stream << '.';
		stream << ver.minor;
		stream << '.';
		stream << ver.revision;
		stream << '.';
		stream << ver.build;
		return stream;
	}
};

#define SETTINGS_GENERAL_BYTE_COUNT 17
typedef struct  {
  Version version;
  uint16 writeCount;
  uint32 crc; // Calculated over entire flash page. Assumed to be 0 during calculation
  uint8 numPins; // Always store NUM_PINS in flash, but limit display on GUI to numPins
  // Wifi ID, password, and encryption level?

  // When adding fields:
  //   * Increase byte count (SETTINGS_GENERAL_BYTE_COUNT)
  //   * Increase number of fields in JsonDocument capacity below
  uint8 reserved[SETTINGS_GENERAL_SIZE-SETTINGS_GENERAL_BYTE_COUNT];
} SettingsGeneral;
_Static_assert(SETTINGS_GENERAL_SIZE == sizeof(SettingsGeneral), "");
const int SettingsGeneralJsonCapacity = JSON_OBJECT_SIZE(3);

#define NUM_COLORS 5
#define EASY_LED_PIN_BYTE_COUNT 24
struct EasyLEDPin {
  uint8 index;
  uint8 function;
  uint16 pattern;
  uint16 num_leds;
  uint16 offset;
  // In decreasing importance
  CRGB colors[NUM_COLORS]; // rgb
  // When adding fields:
  //   * Increase byte count (EASY_LED_PIN_BYTE_COUNT)
  //   * Increase number of fields in JsonDocument capacity below
  uint8 reserved[EASY_LED_PIN_SIZE - EASY_LED_PIN_BYTE_COUNT];
};
_Static_assert(sizeof(EasyLEDPin) == EASY_LED_PIN_SIZE, "");
// This doesn't technically need to be in the .h file since we aren't exposing it
// to other files, but it makes adding fields to EasyLEDPin easier
//                                   Colors                  rest of fields
const int EasyLEDPinJsonCapacity = 5*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(5);




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
    //Settings() {};

    void read() {
      // Automatically read from eeprom on init
      IO(SETTINGS_IO_READ);
    }

    bool write() {
      return IO(SETTINGS_IO_WRITE);
    }

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
const int SettingsJsonCapacity = SettingsGeneralJsonCapacity + NUM_PINS*EasyLEDPinJsonCapacity;


#endif // _SETTINGS_H_