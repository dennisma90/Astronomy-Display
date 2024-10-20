#include <Arduino.h>
#include <WiFi.h>
#include "credentials.h"
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>


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

void readFile(){
    File file = SPIFFS.open("/data/clouds.json", "r");
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    JsonDocument jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, file);

    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonVariant time = jsonDoc["data_day"]["time"];

    if (time.is<JsonArray>()) {
        JsonArray time_array = time.as<JsonArray>();
        Serial.println("Select a time index:");
        for (int i = 0; i < time_array.size(); i++) {
            Serial.print(i);
            Serial.print(": ");
            Serial.println(time_array[i].as<String>());
        }

        int index;
        Serial.print("Enter the index: ");
        while (!Serial.available()) {}
        index = Serial.parseInt();

        if (index >= 0 && index < time_array.size()) {
            JsonVariant fog_probability = jsonDoc["data_day"]["fog_probability"];
            JsonVariant totalcloudcover_max = jsonDoc["data_day"]["totalcloudcover_max"];
            JsonVariant lowclouds_max = jsonDoc["data_day"]["lowclouds_max"];
            JsonVariant midclouds_mean = jsonDoc["data_day"]["midclouds_mean"];

            if (fog_probability.is<JsonArray>()) {
                JsonArray fog_probability_array = fog_probability.as<JsonArray>();
                Serial.print("Fog Probability: ");
                Serial.println(fog_probability_array[index].as<int>());
            }

            if (totalcloudcover_max.is<JsonArray>()) {
                JsonArray totalcloudcover_max_array = totalcloudcover_max.as<JsonArray>();
                Serial.print("Total Cloud Cover Max: ");
                Serial.println(totalcloudcover_max_array[index].as<int>());
            }

            if (lowclouds_max.is<JsonArray>()) {
                JsonArray lowclouds_max_array = lowclouds_max.as<JsonArray>();
                Serial.print("Low Clouds Max: ");
                Serial.println(lowclouds_max_array[index].as<int>());
            }

            if (midclouds_mean.is<JsonArray>()) {
                JsonArray midclouds_mean_array = midclouds_mean.as<JsonArray>();
                Serial.print("Mid Clouds Mean: ");
                Serial.println(midclouds_mean_array[index].as<int>());
            }
        } else {
            Serial.println("Invalid index");
        }
    }

    file.close();
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

    // Read and print the contents of the file
    readFile();
}

void loop(){}

