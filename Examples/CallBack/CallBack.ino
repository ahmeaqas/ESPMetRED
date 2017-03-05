#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <ESPMetRED.h>

const char* MQTT_PUBLISH_TOPIC = "ESPMetRED/OUT";
const char* MQTT_SUBSCRIBE_TOPIC = "ESPMetRED/IN";
const char* CLIENT_ID = "ESPMetRED";
const char* WIFI_SSID = "Ahmed";
const char* WIFI_PASSWORD = "Password";
const char* MQTT_SERVER = "192.168.0.30";
const char* MQTT_USER = "User_Name";
const char* MQTT_PASSWORD = "Password"; 

ESPMetRED client;

void setup() {
  client.setcallback(callback);
}

void callback(String mqtt_payload)
{
  Serial.println(mqtt_payload);
}

void loop() {
  client.keepalive();

}
