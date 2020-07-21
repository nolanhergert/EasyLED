//Thanks gdsports!
// https://forum.arduino.cc/index.php?topic=499399.msg3408025#msg3408025
//Webserver.send_P(200, "text/html", INDEX_HTML);
// Thanks Jason Coon!
// https://github.com/jasoncoon/esp8266-fastled-webserver/
// Thanks Bitluni!
// https://github.com/bitluni/bitluniHomeAutomation/

#include "index_html.h"
#include "main_fastled.h"

// Don't want this long term.
#include <FastLED.h>


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


#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#define SSID "LightBike"
// No password for now

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

void setup() {
  delay(500);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(SSID);

  // Don't let wifi go to sleep when wifi is on
  // So far don't need it as response time of ~1s is fine
  //WiFi.setSleepMode(WIFI_NONE_SLEEP);

  //Serial.setDebugOutput(true);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Allow http updates
  httpUpdater.setup(&server);

  server.on("/", handleRoot);
  
  server.on("/set", HTTP_GET, []() {
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "brightness") {
        FastLED.setBrightness(server.arg(i).toInt());
      }
    }
    server.send(200);
  });

  server.onNotFound(handleNotFound);


  server.begin();
  Serial.println("HTTP server started");

  setup_FastLED();
  
}

void loop() {
  server.handleClient();
  loop_FastLED();
}