/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//Thanks gdsports!
// https://forum.arduino.cc/index.php?topic=499399.msg3408025#msg3408025
//Webserver.send_P(200, "text/html", INDEX_HTML);
// Thanks Jason Coon!
// https://github.com/jasoncoon/esp8266-fastled-webserver/
// Thanks Bitluni!
// https://github.com/bitluni/bitluniHomeAutomation/



#include <Arduino.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DNSServer.h>
#include <iostream>
#include <map>
#include <string>

#include "index.html.h"
#include "main_fastled.h"
#include "settings.h"
#include "common.h"

#include <EEPROM.h>

#define SSID_PREFIX "EasyLED"
#define DNS_NAME "easyled.local"
#define WIFI_STARTUP_TIMEOUT_SECS 120
#define LED_BUILTIN_BREATHE_PERIOD_MILLISECS 6123
#define LED_BUILTIN_PULSE_PERIOD_MILLISECS 1000
#define LED_BUILTIN_PULSE_WIDTH_MILLISECS 50


// No password for now


struct Settings settings;
// Used to determine if we should shut off the wifi
boolean wifiMainPageAccessed = false;
boolean wifiTimeoutElapsed = false;

DNSServer dnsServer;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

// Here for now since it has appendages to multiple modules
void ParsePinVariable(EasyLEDPin *pin, String argName, String argValue) {
  /*
  Serial.print("Pin: ");
  Serial.print(pin->index);
  Serial.print(" Parsing: ");
  Serial.print(argName);
  Serial.print(", ");
  Serial.println(argValue);
  */
  if (argName == "function") {
    pin->function = argValue.toInt();
    // TODO: Switch on function
  } else if (argName == "num_leds") {
    pin->num_leds = argValue.toInt();
    ModifyLedStrip(pin);
  } else if (argName == "pattern") {
    pin->pattern = argValue.toInt();
  } else if (argName == "color0") {
    // Thanks Michael! https://stackoverflow.com/a/3409211/931280
    // WARNING: no sanitization or error-checking whatsoever

    int pos = 0;
    for (int count = 0; count < 3; count++) {
      sscanf(&(argValue[pos]), "%2hhx", &(pin->colors[0].raw[count]));
      pos += 2;
    }
  } else {
    Serial.print("Match not found for: ");
    Serial.println(argName);
  }
}

void ParseURLArgs(ESP8266WebServer& server) {
  // Some properties are general, others are parts of a pin
  // And everything needs to get updated *live*, so having trouble
  // deciding where to put everything so that it's flexible long-
  // term.
  uint8_t pin = 0;
  Serial.println(server.argName(0));

  if (server.argName(0) == "brightness") {
    settings.general.brightness = server.arg(0).toInt();
    FastLED.setBrightness(settings.general.brightness);
  } else if (server.argName(0) == "save") {
    Serial.println("Attempting save!");
    if (RC_FAILURE == settings.save()) {
      // Return an error to server?!
      Serial.println("Unsuccessful in writing to eeprom");
    }
  } else if (server.argName(0) == "reset") {
    Serial.println("Attempting reset!");
    settings.setDefaults();
    if (RC_FAILURE == settings.save()) {
      // Return an error to server?!
      Serial.println("Unsuccessful in writing to eeprom");
    }
    // How much do we want to reset? Wifi? Separate function?
    // For now just do fast led
    setup_FastLED(&settings);

  } else if (server.argName(0) == "pin") {
    pin = server.arg(0).toInt();
    for (int i = 1; i < server.args(); i++) {
      ParsePinVariable(&(settings.pins[pin]), server.argName(i), server.arg(i));
    }
  } else {
    Serial.print("Invalid argument: ");
    Serial.println(server.argName(0));
  }
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

uint8_t pin = 0;
void setup() {

  Serial.begin(115200);
  Serial.println();


  // Read in initial settings
  settings.read();
  Serial.print("Configuring access point...");
  // You can remove the password parameter if you want the AP to be open.

  String mac = WiFi.macAddress();
  mac.replace(":","");
  String ssid = SSID_PREFIX + mac.substring(8);
  // Append latter mac address bytes onto the end for a persistent non-colliding ssid name
  Serial.println(ssid);
  WiFi.softAP(ssid);


  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, DNS_NAME, WiFi.softAPIP());



  // Allow http updates
  httpUpdater.setup(&server);

  server.on("/", []() {
    // Send the static webpage generated from index.html
    // and stored in flash
    server.send_P(200, "text/html", INDEX_HTML);
    wifiMainPageAccessed = true;
  });

  server.on("/settings.json", []() {
    // Send a JSON file of our current settings
    Serial.println("Received request for json");
    String json;
    settings.serialize(json);
    //Serial.println(json);
    server.send(200, "text/json", json);
  });

  // Using PATCH so we can be RESTful and be clear that we don't want to
  // add new instances on the server or whatever. Want to be able to send
  // a partial and full update of settings
  server.on("/body", HTTP_PATCH, []() {
    //server.
    //DeserializationError err = deserializeJson(doc, input);
          if (server.hasArg("plain")== false){ //Check if body received

            server.send(200, "text/plain", "Body not received");
            return;

      }
      String json;
      settings.serialize(json);
      String message = "Body received:\n";
             // What the server sent to us as json
             message += server.arg("plain");
             message += "\n";
             message += json;

      server.send(200, "text/plain", message);
  });


  server.on("/set", HTTP_GET, []() {
    // Very simple parsing for now
    server.send(200);
    ParseURLArgs(server);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  setup_FastLED(&settings);

  pinMode(LED_BUILTIN, OUTPUT);
  // Keep on-board led switched on
  digitalWrite(LED_BUILTIN, LOW);

}

int16_t led_builtin_val = 0;
int16_t led_builtin_breathe_val = 0;
uint16_t ms_after_heartbeat = 0;
int8_t sign = 1;
int16_t change = 0;
uint64_t prevMillis = 0;
void loop() {

  server.handleClient();

  dnsServer.processNextRequest();
  loop_FastLED(&settings);

  EVERY_N_SECONDS(1) {
    // Only want to run the below code if the timeout is not elapsed
    if (wifiTimeoutElapsed == false) {
      if (millis() > (WIFI_STARTUP_TIMEOUT_SECS * 1000)) {
        wifiTimeoutElapsed = true;
        if (wifiMainPageAccessed == false) {
          // Turn off wifi
          Serial.println("Disabling wifi after 120 seconds of unuse at startup");
          WiFi.mode( WIFI_OFF );
          WiFi.forceSleepBegin();
          delay( 1 );
        }
      }
    }
  }



/*
  // Set the on-board blue LED a-breathin
  // Requires pin 4 to not be used by FastLED
  // Also can't support AnalogWrite and 8 pins of FastLED at the same time for right now
  // https://www.reddit.com/r/FastLED/comments/kl13bn/text_will_not_fit_in_region_iram1_0_seg/
  // So not doing fading for now;
  if (settings.pins[3].num_leds == 0) {
       // Not sure why the extra divide by 2 is required at the end
    change = sign * (millis() - prevMillis) * (PWMRANGE*2) / (LED_BUILTIN_BREATHE_PERIOD_MILLISECS);
    // Handle wraparound in an absolute value sense (bounce off the edge with same velocity)
    led_builtin_breathe_val -= change;
    if (led_builtin_breathe_val < 0) {
      sign = 1;
      led_builtin_breathe_val = abs(led_builtin_breathe_val);
    } else if (led_builtin_breathe_val >= PWMRANGE) {
      sign = -1;
      led_builtin_breathe_val = PWMRANGE - (led_builtin_breathe_val - PWMRANGE);
    }

    // Write out final value
    ms_after_heartbeat = millis() % LED_BUILTIN_PULSE_PERIOD_MILLISECS;
    if (ms_after_heartbeat > 0 && ms_after_heartbeat < LED_BUILTIN_PULSE_WIDTH_MILLISECS) {
      // Heartbeat
      //led_builtin_val = 0; // Max brightness is 0
    } else {
      // Breathe val
      led_builtin_val = led_builtin_breathe_val;
    }

    analogWrite(LED_BUILTIN, led_builtin_val);
    prevMillis = millis();
    // Also analogWriteFreq(); Default is 1KHz
  }
  */


}