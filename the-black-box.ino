/*
2025-08-01

"The Black Box" is a smart "energy aware" device developed during the 11th edition of PIFcamp in Soča, Slovenia: https://pif.camp/sl/
after the Energy Awareness workshop held by Bernhard Rasinger: https://pif.camp/pifcamp-s11-e01-god-of-light-intro/

The following code shows how to request Carbon intensity data from the web app electricitymaps.com and use it to control a relay connected to the main grid.
To make it work you first need to sign in and get an API key, this coded uses the free tier, further info on: https://portal.electricitymaps.com/developer-hub/api/getting-started#free-tier

The device runs on the ESP8266 and uses a 5VDC relay (rated 250V 10A) and a WS2812 addressable LED.

Diagrams, 3D models and additional files are available in the repository: https://github.com/stziopa/the-black-box

*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <base64.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

const int relayPin = D1; //relay control pin

const int ledPin = D4; // WS2812B data line pin 
const int numLeds = 1; // Number of LEDs in the strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numLeds, ledPin, NEO_GRB + NEO_KHZ800);

unsigned long long lastCall; //time of the last request sent to the server (ms)
int interval = 60000; //how often we send a new request (60000 ms = 1 minute)

const char* ssid = "NETWORK_NAME"; //replace with your wifi network name 
const char* password = "PASSWORD"; //and password (leave brackets empty if network is open)

//the API url, get it from the developer hub playground: https://portal.electricitymaps.com/developer-hub/playground
const char* apiUrlLatest = "https://api.electricitymaps.com/v3/carbon-intensity/latest?zone=SI";
const char* apiUrlHistory = "https://api.electricitymaps.com/v3/carbon-intensity/history?zone=SI";
const char* username = "YOUR_SIGNIN_EMAIL"; 
const char* authPassword = "YOUR_API_KEY";

WiFiClientSecure client;
HTTPClient http;

int carbonIntensityLatest; //latest Carbon intensity value
int history_item_carbonIntensity; //past Carbon intensity value

//send API request for the latest Carbon intensity value
void requestLatest() {
    if (WiFi.status() == WL_CONNECTED) {
    String auth = username + String(":") + authPassword;
    String authEncoded = base64::encode(auth);
    http.begin(client, apiUrlLatest);
    http.addHeader("Authorization", "Basic " + authEncoded);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String response = http.getString();
      //Serial.println("API Response:");
      //Serial.println(response);

      DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    String carbonIntensityLatestString = doc["carbonIntensity"].as<String>();
    String datetimeLatestString = doc["datetime"].as<String>();

    //Convert String to integer
    carbonIntensityLatest = atoi(carbonIntensityLatestString.c_str());

    Serial.println("Latest values:");
    Serial.println(carbonIntensityLatest);
    Serial.println(datetimeLatestString);

    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
}

//send API request for past Carbon intensity value (24 hours)
void requestHistory() {
    if (WiFi.status() == WL_CONNECTED) {
    String auth = username + String(":") + authPassword;
    String authEncoded = base64::encode(auth);
    http.begin(client, apiUrlHistory);
    http.addHeader("Authorization", "Basic " + authEncoded);
    int httpCode = http.GET();
    if (httpCode > 0) {
      String response = http.getString();
      //Serial.println("API Response:");
      //Serial.println(response);

      DynamicJsonDocument doc(6144);
        DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

      //get the array of the past 24 hours history
      JsonArray history_item = doc["history"].as<JsonArray>();
        history_item_carbonIntensity = history_item[0]["carbonIntensity"]; //read only first item [0] which corresponds to 24 hours before
        const char* history_item_datetime = history_item[0]["datetime"];

        Serial.println("history values:");
        Serial.println(history_item_carbonIntensity);
        Serial.println(history_item_datetime);

    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
}

//here we compare the latest Carbon intensity value against yesterday's same time
void comparation() {
  if(carbonIntensityLatest >= history_item_carbonIntensity) {
    //we set the pin to LOW = relay OPEN = NO CURRENT THROUGH
    digitalWrite(relayPin, LOW);
    setColor(255, 0, 0); //set LED to RED
    Serial.println("We are consuming more than yesterday :(");
  } else {
    //we set the pin to LOW = relay CLOSED = CURRENT FLOWS :)
    digitalWrite(relayPin, HIGH);
    setColor(0, 255, 0); //set LED to GREEN
    Serial.println("We are consuming less than yesterday :)");
  }
}

void setup() {
  Serial.begin(115200);

  //set relay to OPEN by default, the AC LEAD wire is wired to NO (normally open) terminal
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  //initialize LED 
  strip.begin();
  setColor(255, 255, 255);
  strip.show();

  //establish Wifi connection 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    //blink LED blue color while connecting
    setColor(0, 0, 255);
    strip.show();
    delay(500);
    Serial.println("Connecting to WiFi...");
    setColor(0, 0, 0);
    strip.show();
    delay(500);
  }
  Serial.println("Connected to WiFi");

  client.setInsecure(); // Disable HTTPS certificate validation

  //Send the first API request
  requestHistory();
  requestLatest();
  comparation();
}

void loop() {

  //send a new request
  if(millis()>(lastCall+interval)) {

    setColor(0, 0, 255);
    strip.show();
    delay(500);
    Serial.println("New request sent...");
    setColor(0, 0, 0);
    strip.show();
    delay(500);

    requestHistory();
    requestLatest();
    comparation();
    lastCall=millis();
  }

}

//update LED status
void setColor(int r, int g, int b) {
  for (int i = 0; i < numLeds; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}
