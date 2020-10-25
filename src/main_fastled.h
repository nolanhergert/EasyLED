
#ifndef _MAIN_FASTLED_H_
#define _MAIN_FASTLED_H_

#include <FastLED.h>
#include <settings.h>

void setup_FastLED(const Settings *pSettings);
void loop_FastLED();

void ModifyLedStrip(int pin, int length, int pattern, CRGB colors[5]);

#endif // _MAIN_FASTLED_H_