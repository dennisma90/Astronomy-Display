#include <Arduino.h>
#include <WiFi.h>
#include "credentials.h"
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <TimeLib.h>



/**
 * @brief Connects the ESP32 to a WiFi network
 * 
 * This function sets the WiFi mode to station mode and initiates a connection
 * to the specified WiFi network using the provided SSID and password. It
 * continuously checks the connection status and prints dots to the serial monitor
 * until the connection is established. Once connected, it prints a success message
 * and the local IP address of the ESP32.
 */
void connectToWifi(){
    WiFi.mode(WIFI_STA); //Optional
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

/**
 * @brief Fetches data from the API and saves it to a file
 * 
 * Uses the HTTPClient library to make a GET request to the API and retrieve the data.
 * Then opens a file on the SPIFFS file system and writes the data to it.
 * If the file cannot be opened, an error message is printed to the serial monitor.
 * If the GET request fails, an error message with the HTTP status code is printed to the serial monitor.
 * 
 * @return void
 */
void fetchAndSaveData(){
    HTTPClient http;
    http.begin(apiUrl);
    int httpResponseCode = http.GET();

    if(httpResponseCode>0){
        String payload = http.getString();
        File file = SPIFFS.open("/data/clouds.json", "w");
        if(!file){
            Serial.println("Failed to open file for writing");
            return;
        }
        file.print(payload);
        file.close();
        Serial.println("Data saved to file");
    }
    else{
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

/**
 * @brief Extracts values from /data/clouds.json and manipulates them
 */
void manipulateCloudsData()
{
    // Open the JSON file
    File file = SPIFFS.open("/data/clouds.json", "r");
    if (!file)
    {
        Serial.println("Failed to open file for reading");
        return;
    }

    // Parse the JSON file
    JsonDocument jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, file);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonVariant day_time = jsonDoc["data_day"]["time"];
    JsonVariant day_sunset = jsonDoc["data_day"]["sunset"];
    JsonVariant day_sunrise = jsonDoc["data_day"]["sunrise"];
    JsonVariant xmin_time = jsonDoc["data_xmin"]["time"];

    // Create the list
    String night_duration[2];
    night_duration[0] = day_time[0].as<String>() + " " + day_sunset[0].as<String>();
    night_duration[1] = day_time[1].as<String>() + " " + day_sunrise[1].as<String>();
    
    // Print the list
    Serial.println(night_duration[0]);
    Serial.println(night_duration[1]);

    time_t night_start, night_end;
    struct tm tm = {0}; // Initialize the tm struct to zero

    strptime(night_duration[0].c_str(), "%Y-%m-%d %H:%M", &tm);
    tm.tm_year -= 1900; // Adjust the year to be relative to 1900
    tm.tm_mon -= 1; // Adjust the month to be 0-based
    night_start = mktime(&tm);

    strptime(night_duration[1].c_str(), "%Y-%m-%d %H:%M", &tm);
    tm.tm_year -= 1900; // Adjust the year to be relative to 1900
    tm.tm_mon -= 1; // Adjust the month to be 0-based
    night_end = mktime(&tm);

    Serial.println(night_start);
    Serial.println(night_end);

    time_t time_diff = night_end - night_start;
    int hours = time_diff / 3600;
    int minutes = (time_diff % 3600) / 60;

    Serial.print("Time difference: ");
    Serial.print(hours);
    Serial.print(" hours and ");
    Serial.print(minutes);
    Serial.println(" minutes");
}

void setup(){
    Serial.begin(115200);
    delay(1000);

    // Connect to WiFi
    connectToWifi();

    // Mount SPIFFS file system
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
        return;
    }

    // Fetch data from API and save it to a file
    // fetchAndSaveData();

    manipulateCloudsData();

}

void loop(){}

