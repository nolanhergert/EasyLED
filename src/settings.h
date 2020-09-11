#ifndef _SETTINGS_H_
#define _SETTINGS_H_


#include <c_types.h>
#include <FastLED.h>
#include <inttypes.h>

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

typedef struct  {
  Version version;
  uint16 write_count;
  uint32 crc; // Calculated over entire flash page. Assumed to be 0 during calculation
  // Wifi ID, password, and encryption level?
  uint8 reserved[SETTINGS_GENERAL_SIZE-16];
} SettingsGeneral;
_Static_assert(SETTINGS_GENERAL_SIZE == sizeof(SettingsGeneral), "");

#define NUM_COLORS 5
typedef struct {
  uint8 index;
  uint8 function;
  uint16 pattern;
  uint16 num_leds;
  uint16 offset;
  // In decreasing importance
  CRGB colors[NUM_COLORS]; // rgb

  uint8 reserved[EASY_LED_PIN_SIZE - 24];
} EasyLEDPin;
_Static_assert(sizeof(EasyLEDPin) == EASY_LED_PIN_SIZE, "");



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


#endif // _SETTINGS_H_