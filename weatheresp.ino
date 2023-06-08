#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SSD1306.h"
#include <Adafruit_GFX.h>

#define SDA_PIN 5
#define SCL_PIN 4
#define SSD_ADDRESS 0x3c

SSD1306  display(SSD_ADDRESS, SDA_PIN, SCL_PIN);

const char* ssid = "Z.A. Studios @ 2.4GHz";
const char* password = "lastkiss";

// Change the URL to the API endpoint of your weather service, and add your API key
const char* weatherApiUrl = "https://api.openweathermap.org/data/2.5/weather?lat=43.5945&lon=-83.8889&appid=385fc58ed4e45f788fce1111b20e6cac";

// Set the desired weather conditions
const float minTemperature = 14.0;
const float maxTemperature = 104.0;
const float minWind = 00.0;
const float maxWind = 20.0;
const float minVisibility = 3.0;
int attempts = 0;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    attempts = attempts + 1;
    if (attempts >= 5) {
      ESP.restart();
    }
  }
  Serial.println("Connected to WiFi");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Send HTTP request to weather API
    HTTPClient http;
    http.useHTTP10(true);
    http.begin(weatherApiUrl);
    int httpCode = http.GET();
    if (httpCode > 0) {
      // Parse JSON response from weather API
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, http.getStream());
      /*if (error) {
        Serial.print("Error parsing JSON: ");
        Serial.println(error.c_str());
        return;
        } */
      // Get temperature and humidity values from JSON response
      double temperature = doc["main"]["feels_like"];
      double wind = doc["wind"]["speed"];
      int visibility = doc["visibility"];
      const char* precipitation = doc["weather"][0]["main"];
      
      temperature = temperature - 273.15;
      temperature = temperature * 1.8;
      temperature = temperature + 32;
      wind = wind * 2.237;
      visibility = visibility / 1000;
      visibility = visibility / 1.609;
      
      String Temp = String(temperature);
      String Wind = String(wind);
      String Visibility = String(visibility);

      Serial.println(Temp + "f");
      Serial.println(Wind + "mph");
      Serial.println(Visibility + "SM");
      Serial.println(precipitation);

      // Check if all weather conditions are within the desired range
      if (temperature >= minTemperature && temperature <= maxTemperature &&
          wind >= minWind && wind <= maxWind &&
          visibility >= minVisibility &&
          precipitation != "Rain"  && precipitation != "Snow" && precipitation != "Sleet" && precipitation != "Hail" && precipitation != "Freezing_Rain" && precipitation != "Mist") {
        Serial.println("All weather conditions are within the desired range");
        display.setFont(Lato_Hairline_40);
        display.clear();
        display.drawString(0,0, "Go Fly!");
        display.display();
        display.setFont(ArialMT_Plain_10);
        display.drawString(0,54, Temp+"-"+Wind+"-"+Visibility+"-"+precipitation);
        display.display();
        
      } else {
        Serial.println("Not all weather conditions are within the desired range");
        display.clear();
        display.setFont(ArialMT_Plain_10);
        display.drawString(0,54, Temp+"-"+Wind+"-"+Visibility+"-"+precipitation);
        display.display();
      }
    }
    http.end();
  }

  // Wait for 5 minutes before sending another request to the weather API
  delay(5 * 60 * 1000);
}
