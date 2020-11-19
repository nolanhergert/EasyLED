
#ifndef _MAIN_FASTLED_H_
#define _MAIN_FASTLED_H_

#include <FastLED.h>
#include <settings.h>

void setup_FastLED(const Settings *pSettings);
void loop_FastLED(const Settings *pSettings);

void ModifyLedStrip(const EasyLEDPin *pPin);

#endif // _MAIN_FASTLED_H_