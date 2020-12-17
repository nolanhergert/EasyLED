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



#include <FastLED.h>

#include <Arduino.h>
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

#define SSID "EasyLED"
#define DNS_NAME "easyled.local"
// No password for now


struct Settings settings;

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
  uint8 pin = 0;
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

uint8 pin = 0;
void setup() {

  Serial.begin(115200);
  Serial.println();


  // Read in initial settings
  settings.read();
  Serial.print("Configuring access point...");
  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(SSID);


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
  digitalWrite(LED_BUILTIN, HIGH);


}

uint16 b = 0;
int8 inc = 1;
void loop() {

  server.handleClient();

  dnsServer.processNextRequest();
  loop_FastLED(&settings);


/*
  // Simple pulse for main blue LED
  // Requires pin 4 to not be used by FastLED
  if (b == PWMRANGE) {
    inc = -1;
  } else if (b == 0) {
    inc = 1;
  }
  analogWrite(LED_BUILTIN,b += inc);
  // Also analogWriteFreq(); Default is 1KHz
  delay(2);

  EVERY_N_MILLISECONDS( 1000 ) {
    Serial.println("Hello!");
  }
  */

}