#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

//Wifi
ESP8266WiFiMulti WiFiMulti;


// Define NTP Client to get time


//Time
const long utcOffsetInSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

const int beerPin = 2;
int buttonState = 0; 

void setup() {
  Serial.begin(9600);
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("GWS-Besucher", "%dGWS-BW1an");
  Serial.println("[SETUP] Wifi initialized");
  timeClient.begin();
  Serial.println("[SETUP] Time initialized");
}

void loop() {
  Serial.println(buttonState);
  if (digitalRead(beerPin) == LOW){
    sendMessage();
  }
}


void sendMessage() {
  timeClient.update();

  if ((WiFiMulti.run() == WL_CONNECTED)) {

    int hour = timeClient.getHours();
    String type = "beer";

    Serial.println(String(hour));
    
    if (hour < 12) {
      type = "coffee";
    } else if (hour == 12) {
      type = "lunch";
    }

    String url1 = "https://prod-41.westeurope.logic.azure.com/workflows/b8750ce167b64a9aadae15b5321ecf4c/triggers/manual/paths/invoke/" + String(hour);
    String url2 = "?api-version=2016-06-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=Rd2ROwC0idCELODFpUKz1ZtZmCH-vIdc-hUEdlU6hIM";
    String request = url1 + url2;

                                    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, request)) {  // HTTPS

      Serial.print("[HTTPS] POST...\n");
      // start connection and send HTTP header
      int httpCode = https.POST("");

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
}
