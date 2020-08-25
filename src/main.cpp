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

#include "index_html.h"
#include "main_fastled.h"

#include <main.h>
#include <FastLED.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include <ESP8266mDNS.h>

#include <iostream>
#include <map>
#include <string>

#define SSID "EasyLED"
// No password for now


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

struct Cache {
  uint16 version;
  EasyLEDPin pins[8];
};

Cache cache;


void UpdateLedStrip(EasyLEDPin *pin) {
  ModifyLedStrip(pin->index, pin->num_leds, pin->pattern, pin->colors);
}

void ParsePinArg(EasyLEDPin *pin, String argName, String argValue) {
  if (argName == "function") {
    pin->function = argValue.toInt();
    // TODO: Switch on function
  } else if (argName == "num_leds") {
    pin->num_leds = argValue.toInt();
    UpdateLedStrip(pin);
  } else if (argName == "pattern") {
    pin->pattern = argValue.toInt();
    UpdateLedStrip(pin);
  } else if (argName == "color0") {
    // Thanks Michael! https://stackoverflow.com/a/3409211/931280
    /* WARNING: no sanitization or error-checking whatsoever */
    
    int pos = 0;
    for (int count = 0; count < 3; count++) {
      // 1 = Skip the "#" symbol
      sscanf(&(argValue[1+pos]), "%2hhx", &(pin->colors[0].raw[count]));
      pos += 2;
    }
    UpdateLedStrip(pin);
    //pin->colors[0].red = std::stoul(argValue[1], nullptr, 16);
  }
    else {
    Serial.print("Match not found for: ");
    Serial.println(argName);
  }
}

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.
*/


void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
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

uint8 pin = 0;
void setup() {
  delay(50);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(SSID);

  for (int i = 0; i < NUM_PINS; i++) {
    cache.pins[i].index = i;
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

    // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  MDNS.begin("easyled");
  Serial.println("mDNS responder started");


  // Allow http updates
  httpUpdater.setup(&server);

  server.on("/", handleRoot);
  
  server.on("/set", HTTP_GET, []() {
    // Very simple parsing for now
    pin = server.arg(0).toInt();
    Serial.println(pin);
    for (int i = 1; i < server.args(); i++) {
      ParsePinArg(&(cache.pins[pin]), server.argName(i), server.arg(i));
    }
    server.send(200);
  });

  server.onNotFound(handleNotFound);


  server.begin();
  Serial.println("HTTP server started");

    // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  setup_FastLED();
  
}

uint16 b = 0;
int8 inc = 1;
void loop() {
  server.handleClient();
  MDNS.update();
  loop_FastLED();

/*
  // Simple pulse for main blue LED
  if (b == PWMRANGE) {
    inc = -1;
  } else if (b == 0) {
    inc = 1;
  }
  analogWrite(LED_BUILTIN,b += inc);
  // Also analogWriteFreq(); Default is 1KHz
  delay(2);
  */
  

}