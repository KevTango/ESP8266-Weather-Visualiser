/* This program connects to the WiFi and extracts weather data from https://openweathermap.org/ to output into NeoPixels
 *  Created by: Kevin Tang
 *  Date last modified: 15/02/2019
 */

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <string.h>
#define second 1000 // ms in a second
#define hour 3600000 // ms in an hour
#define baud 115200 // Baud Rate
#define PIN D6 // Which pin on the ESP8266 is connected to the NeoPixels?
#define NUMPIXELS 12 // Number of LEDs on NeoPixel Ring?
#define OLED_RESET LED_BUILTIN  //4

Adafruit_SSD1306 display(OLED_RESET);

const char* ssid     = "SSID"; // Input SSID
const char* password = "PASSWORD"; // Input Password
const char* website_name = "api.openweathermap.org"; // Website name

const int httpPort = 80;
const char* link = "/data/2.5/weather";
const char* location = "2193733"; // After inputting city into box, the location is from the set of numbers at the end
const char* api_key = "API_KEY"; // Input API Key
const char* measurement = "metric"; // Metric Measurement
// const char* measurement = "imperial"; // Uncomment for imperial measurement

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 500; // delay for half a second

// Connecting to the WiFi
void setup() {
  delay(second);
  Serial.begin(baud); // Baud rate for data transmission
  WiFi.mode(WIFI_AP); 
  Serial.println(); // Prints empty line
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // Connects to the WiFi using the login details

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  buffer_clear();
  
  while (WiFi.status() != WL_CONNECTED) { // If WiFi is not connected
    delay(second);
    Serial.println("Trying to reconnect");
    display.setCursor(0,0);
    display.println("WIFI not connected");
    display.display();
  }

  Serial.println();
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Prints IP Address
  pixels.begin(); // Initialises the NeoPixel library (Currently messes up Serial Monitor)
}

// Clears the buffer
void buffer_clear() {
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
}


// Updates the weather every hour
void loop() {
  updateWeather();
  delay(hour);
  //delay(5000);
}

// Capitalises the first letter of each word
String capitalise_weather_description(String words) {
  return words;
}

void updateWeather() {
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(website_name);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(website_name, httpPort)) {
    Serial.println("Connection failed");
    return;
  }

  // Creating the URL
  String url = String(link) + "?id=" + String(location) + "&appid=" + String(api_key) + "&units=" + String(measurement)/*+ ".json"*/;
  Serial.print("Extracting from: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + website_name + "\r\n" +
               "User-Agent: ESP8266\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println("Request Sent");
  delay(second);

  // Check HTTP status
  String line = client.readStringUntil('\n');
  Serial.println();
  Serial.println(line);
  
  if (line != "HTTP/1.1 200 OK\r") {
    Serial.print("Unexpected response: ");
    Serial.println(line);
    return;
  }

  // Skip HTTP Header
  if (!client.find("\r\n\r\n")) {
    Serial.println("Invalid response");
    return;
  }

  // Allocate JSON Buffer
  DynamicJsonBuffer jsonBuffer(4094); // If your Arduino JSON library is 6.XX, then DynamicJsonBuffer will not work. Downgrade the library to 5.XX to fix this issue
  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  
  if (!root.success()) {
    Serial.println("Parsing failed!");
    return;
  }

  buffer_clear();
  
  // Extract weather values
  const String weather = root["weather"][0]["main"];
  String weather_description = root["weather"][0]["description"];
  //String capitalised_description = capitalise_weather_description(weather_description);
  const String temperature = root["main"]["temp"];
  const String location = root["name"];
  const String country = root["sys"]["country"];
  Serial.println("Weather: " + weather_description);
  Serial.println("Current Temperature: " + temperature + "°");
  Serial.println("Location: " + location + ", " + "country");

  display.setCursor(0,0);
  display.println("Weather: " + weather_description);
  display.println("Current Temp: " + temperature + (char)247);
  display.println("City: " + location + ", " + country);
  display.display();

  // Configuring the NeoPixel depending on weather
  // Thunderstorm
  if(weather == "Thunderstorm") {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // Red
      pixels.show(); // This sends the updated pixel color to the hardware
    }
  }
  
  // Drizzle
  if(weather == "Drizzle") {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 165, 20)); // Orange
      pixels.show(); // This sends the updated pixel color to the hardware
    }
  }
  
  // Rain
  if(weather == "Rain") {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 0)); // Yellow
      pixels.show(); // This sends the updated pixel color to the hardware
    }
  }
  
  // Snow
  if(weather == "Snow") {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // Green
      pixels.show(); // This sends the updated pixel color to the hardware
    }
  }
  
  // Atmosphere
  if(weather == "Atmosphere") {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Black
      pixels.show(); // This sends the updated pixel color to the hardware
    }
  }
  
  // Clear
  if(weather == "Clear") {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(135, 206, 235)); // Sky Blue
      pixels.show(); // This sends the updated pixel color to the hardware
    }
  }
  
  // Clouds
  if(weather == "Clouds") {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(128, 0, 128)); // Purple
      pixels.show(); // This sends the updated pixel color to the hardware
    }
  }
}
