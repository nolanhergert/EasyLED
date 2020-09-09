

// Index is 0-7, but pin number on the board and GUI is 1-8
#define NUM_PINS 8
#define NUM_COLORS 5

typedef struct {
  uint8 index;
  uint8 function;
  uint16 pattern;
  uint16 num_leds;
  uint16 offset;
  // In decreasing importance
  CRGB colors[NUM_COLORS]; // rgb

  //128B total?
  uint8 reserved[100];
} EasyLEDPin;

