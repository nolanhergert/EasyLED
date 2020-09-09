#include <c_types.h>
#include <FastLED.h>

////////////////////////////////////////////////////////////////
// Helpful compile time print if the below assertions fail
// https://stackoverflow.com/a/35261673/931280
//char (*__kaboom)[offsetof(Settings, pins)] = 1;
////////////////////////////////////////////////////////////////


#define SETTINGS_GENERAL_SIZE 512
#define FLASH_MAX_PAGE_WRITE_COUNT 10000
typedef struct  {
  uint16 version;
  uint16 write_count;
  // Wifi ID, password, and encryption level?
  uint8 reserved[SETTINGS_GENERAL_SIZE-4];
} SettingsGeneral;

_Static_assert(SETTINGS_GENERAL_SIZE == sizeof(SettingsGeneral), "");

#define EASY_LED_PIN_SIZE 256
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


#define FLASH_PAGE_SIZE 4096
// Index is 0-7, but pin number on the board and GUI is 1-8
#define NUM_PINS 8
typedef struct  {
  SettingsGeneral general;
  EasyLEDPin pins[8];
} Settings;

_Static_assert(sizeof(Settings) < FLASH_PAGE_SIZE, "");
_Static_assert(offsetof(Settings, pins) == SETTINGS_GENERAL_SIZE, "");