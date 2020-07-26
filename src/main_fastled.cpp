
#include <Arduino.h>
#include <FastLED.h>

#include "main_fastled.h"

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
#define TOTAL_NUM_LEDS    320 // just picking a number
#define NUM_STRIPS 8
uint32 offsets[NUM_STRIPS];
uint32 lengths[NUM_STRIPS];
CRGB leds[TOTAL_NUM_LEDS];


#define BRIGHTNESS         255 // can be dynamically changed though
#define FRAMES_PER_SECOND  120




uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void rainbow(CRGB LedsSubset[], uint32 LedsSubsetCount) {
  fill_rainbow( LedsSubset, LedsSubsetCount, gHue, 7);
}

void glitter(CRGB LedsSubset[], uint32 LedsSubsetCount, fract8 chanceOfGlitter) {
  if( random8() < chanceOfGlitter) {
    LedsSubset[ random16(LedsSubsetCount) ] += CRGB::White;
  }
}


void rainbowWithGlitter(CRGB LedsSubset[], uint32 LedsSubsetCount) 
{
  // FastLED's built-in rainbow generator
  rainbow(LedsSubset, LedsSubsetCount);
  // Plus glitter!
  glitter(LedsSubset, LedsSubsetCount, 80);

}


void confetti(CRGB LedsSubset[], uint32 LedsSubsetCount) 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( LedsSubset, LedsSubsetCount, 10);
  int pos = random16(LedsSubsetCount);
  LedsSubset[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon(CRGB LedsSubset[], uint32 LedsSubsetCount)
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( LedsSubset, LedsSubsetCount, 20);
  int pos = beatsin16( 13, 0, LedsSubsetCount-1 );
  LedsSubset[pos] += CHSV( gHue, 255, 192);
}



void bpm(CRGB LedsSubset[], uint32 LedsSubsetCount)
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( uint32 i = 0; i < LedsSubsetCount; i++) { //9948
    LedsSubset[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle(CRGB LedsSubset[], uint32 LedsSubsetCount) {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( LedsSubset, LedsSubsetCount, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    LedsSubset[beatsin16( i+7, 0, LedsSubsetCount-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}



CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(OceanColors_p);
void fillnoise8(CRGB LedsSubset[], uint32 LedsSubsetCount) {

  #define scale 30                                                          // Don't change this programmatically or everything shakes.
  
  for(uint32 i = 0; i < LedsSubsetCount; i++) {                                       // Just ONE loop to fill up the LED array as all of the pixels change.
    uint8_t index = inoise8(i*scale, millis()/10+i*scale);                   // Get a value from the noise function. I'm using both x and y axis.
    LedsSubset[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);    // With that value, look up the 8 bit colour palette value and assign it to the current LED.
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
typedef void (*SimplePatternList[])(CRGB LedsSubset[], uint32 LedsSubsetCount);
//SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, fillnoise8 };
SimplePatternList gPatterns = { fillnoise8, juggle, rainbow, rainbowWithGlitter, confetti, sinelon, bpm };

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}


void RecomputeOffsets() {
  int i = 0;
  // Recompute offsets
  offsets[0] = 0;
  for (i = 1; i < NUM_STRIPS; i++) {
    offsets[i] = offsets[i-1] + lengths[i-1];
  }
} 

// Assume that we entered the new led strips in increasing
// pin order and they are in this linked list in the same
// order
void SetLedStripParameters() {
  CLEDController *pCur = CLEDController::head();
  int i = 0;
	while(pCur) {
    pCur->setLeds(leds + offsets[i], lengths[i]);
    i++;
		pCur = pCur->next();
	}
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

#define TEMP_NUM_LEDS 25

//static const 
void setup_FastLED() {

  int i = 0;
  delay(1000); // 1 second delay for recovery

  
  
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  // Todo: LOAD VALUES FROM EEPROM
  // And save them when changed without wearing out eeprom during testing.
  for (i = 0; i < NUM_STRIPS; i++) {
    lengths[i] = TEMP_NUM_LEDS;
  }
  RecomputeOffsets();
  for (i = 0; i < NUM_STRIPS; i++) {
    AddNewLedStrip(i, offsets[i], lengths[i]);
  }  
}




void loop_FastLED()
{
  int i = 0;

  for (i = 0; i < NUM_STRIPS; i++) {
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[i % ARRAY_SIZE( gPatterns)](&leds[offsets[i]], lengths[i]);
  }


  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  //EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
/*

  EVERY_N_MILLISECONDS( 5 ) {
    // Set all pixels to black
    fadeToBlackBy( leds, TOTAL_NUM_LEDS, 255);
    FastLED.show();

    for (i = 0; i < NUM_STRIPS; i++) {
      lengths[i] = (lengths[i] + 1)%TEMP_NUM_LEDS;
    }
    RecomputeOffsets();

    // No performance difference between addLeds and setLeds, surprisingly
    // And no additional entries on linked list
    for (i = 0; i < NUM_STRIPS; i++) {
      //AddNewLedStrip(i, offsets[i], lengths[i]);
    }
    SetLedStripParameters();
    //Serial.println(ESP.getFreeHeap());
  }
  */
  
}

