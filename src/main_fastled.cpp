
#include <Arduino.h>
#include <FastLED.h>

#include "main_fastled.h"
#include "settings.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define INITIAL_NUM_LEDS 25
#define TOTAL_NUM_LEDS    MAX_PINS*INITIAL_NUM_LEDS
uint32 offsets[MAX_PINS];
uint32 lengths[MAX_PINS];
CRGB leds[TOTAL_NUM_LEDS];


// Actual number of pins from settings
uint8 numPins;


#define FRAMES_PER_SECOND  200


uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void rainbow(const EasyLEDPin *pPin) {
  fill_rainbow( &leds[offsets[pPin->index]], pPin->num_leds, gHue, 7);
}

void glitter(const EasyLEDPin *pPin, fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    leds[offsets[pPin->index] + random16(pPin->num_leds)] += CRGB::White;
  }
}

void rainbowWithGlitter(const EasyLEDPin *pPin) 
{
  // FastLED's built-in rainbow generator
  rainbow(pPin);
  // Plus glitter!
  glitter(pPin, 80);

}


void confetti(const EasyLEDPin *pPin) 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( &leds[offsets[pPin->index]], pPin->num_leds, 10);
  int pos = random16(pPin->num_leds);
  leds[offsets[pPin->index] + pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon(const EasyLEDPin *pPin)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( &leds[offsets[pPin->index]], pPin->num_leds, 20);
  int pos = beatsin16( 13, 0, pPin->num_leds-1 );
  leds[offsets[pPin->index] + pos] += CHSV( gHue, 255, 192);
}

void bpm(const EasyLEDPin *pPin)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( uint i = 0; i < pPin->num_leds; i++) { //9948
    leds[offsets[pPin->index] + i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle(const EasyLEDPin *pPin) {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( &leds[offsets[pPin->index]], pPin->num_leds, 20);
  byte dothue = 0;
  for( uint i = 0; i < 8; i++) {
    leds[offsets[pPin->index] + beatsin16( i+7, 0, pPin->num_leds-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void color(const EasyLEDPin *pPin) {
  for( uint i = 0; i < pPin->num_leds; i++) {
    leds[offsets[pPin->index] + i] = pPin->colors[0];
  }
}



CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(OceanColors_p);
void fillnoise8(const EasyLEDPin *pPin) {

  #define scale 30                                                          // Don't change this programmatically or everything shakes.
  
  for(uint32 i = 0; i < pPin->num_leds; i++) {                                       // Just ONE loop to fill up the LED array as all of the pixels change.
    uint8_t index = inoise8(i*scale, millis()/10+i*scale);                   // Get a value from the noise function. I'm using both x and y axis.
    leds[offsets[pPin->index] + i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);    // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }

  
  EVERY_N_MILLIS(10) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, 48);          // Blend towards the target palette over 48 iterations.
  }
 
  EVERY_N_SECONDS(5) {                                                      // Change the target palette to a random one every 5 seconds.
    uint8_t baseC=random8();
    targetPalette = CRGBPalette16(CHSV(baseC+random8(32), 255, random8(128,255)),   // Create palettes with similar colours.
                                  CHSV(baseC+random8(64), 255, random8(128,255)),
                                  CHSV(baseC+random8(96), 192, random8(128,255)),
                                  CHSV(baseC+random8(16), 255, random8(128,255)));
  }

}





// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])(const EasyLEDPin *pPin);
//SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, fillnoise8 };
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, color, fillnoise8, juggle, confetti, sinelon, bpm };

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))


void RecomputeOffsets() {
  uint i = 0;
  // Recompute offsets
  offsets[0] = 0;
  for (i = 1; i < numPins; i++) {
    offsets[i] = offsets[i-1] + lengths[i-1];
  }
} 

// Assume that we entered the new led strips in increasing
// pin order and they are in this linked list in the same
// order
void SetLedStripParameters() {
  CLEDController *pCur = CLEDController::head();
  uint i = 0;
	while(pCur) {
    pCur->setLeds(leds + offsets[i], lengths[i]);
    i++;
		pCur = pCur->next();
	}
}

void ModifyLedStrip(const EasyLEDPin *pPin) {
  // Most settings will get picked up automatically in
  // loop_FastLED(), but here are a few things that need
  // to get re-initialized
  lengths[pPin->index] = pPin->num_leds;
  
  // Set all leds in global array to black
  FastLED.clear();
  FastLED.show();
  
  RecomputeOffsets();
  SetLedStripParameters();
}

void AddNewLedStrip(int pin, int offset, int length) {
  // I could have used an array here, but that's kind of complicated with templates, etc
  // A solution would be nice, but this *works* for now and is small.
  // Going with simple for now
  //Serial.println("Alloc called");
  switch (pin) {
    case 0:
      FastLED.addLeds<LED_TYPE,D1,COLOR_ORDER>(leds, offset, length).setCorrection(TypicalLEDStrip);
    case 1:
      FastLED.addLeds<LED_TYPE,D2,COLOR_ORDER>(leds, offset, length).setCorrection(TypicalLEDStrip);
    case 2:
      FastLED.addLeds<LED_TYPE,D3,COLOR_ORDER>(leds, offset, length).setCorrection(TypicalLEDStrip);
    case 3:
      FastLED.addLeds<LED_TYPE,D4,COLOR_ORDER>(leds, offset, length).setCorrection(TypicalLEDStrip);
    case 4:
      FastLED.addLeds<LED_TYPE,D5,COLOR_ORDER>(leds, offset, length).setCorrection(TypicalLEDStrip);
    case 5:
      FastLED.addLeds<LED_TYPE,D6,COLOR_ORDER>(leds, offset, length).setCorrection(TypicalLEDStrip);
    case 6:
      FastLED.addLeds<LED_TYPE,D7,COLOR_ORDER>(leds, offset, length).setCorrection(TypicalLEDStrip);
    case 7:
      FastLED.addLeds<LED_TYPE,D8,COLOR_ORDER>(leds, offset, length).setCorrection(TypicalLEDStrip);        
  }
}

uint8 initialStartupBrightness = 0;
uint8 initialStartupBrightnessMax;


//static const 
void setup_FastLED(const Settings *pSettings) {

  int i = 0;
  FastLED.setBrightness(initialStartupBrightness);

  // TODO:
  //FastLED.setMaxPowerInMilliWatts(pSettings->general.maxPowerMilliwatts);

  initialStartupBrightnessMax = pSettings->general.brightness;
  
  numPins = pSettings->general.numPins;
  // And save them when changed without wearing out eeprom during testing.
  for (i = 0; i < numPins; i++) {
    lengths[i] = pSettings->pins[i].num_leds;
    offsets[i] = pSettings->pins[i].offset;
  }

  RecomputeOffsets();
  for (i = 0; i < numPins; i++) {
    AddNewLedStrip(i, offsets[i], lengths[i]);
  }  
}




void loop_FastLED(const Settings *pSettings)
{
  int i = 0;

  // Fade in on startup. Gotta do it :)
  EVERY_N_MILLISECONDS( 50 ) {
    if (initialStartupBrightness < initialStartupBrightnessMax) {
      FastLED.setBrightness(initialStartupBrightness++);
    }
  }
  for (i = 0; i < numPins; i++) {
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[pSettings->pins[i].pattern](&(pSettings->pins[i]));
  }

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay (if needed) to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 
  gHue++;
  
}

