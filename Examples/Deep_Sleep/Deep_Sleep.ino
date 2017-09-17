/*
 * Deep sleep example
   *  Client connects to WiFi within two seconds
   *  Library handles are necessary workarounds for successfull reconnection in short time
 */

#include <ESPMetRED.h>

const char* MQTT_PUBLISH_TOPIC = "ESPMetRED/OUT";
const char* MQTT_SUBSCRIBE_TOPIC = "ESPMetRED/IN";
const char* CLIENT_ID = "ESPMetRED";
const char* WIFI_SSID = "AP"; //Change according to your router ssid
const char* WIFI_PASSWORD = "Password"; //Change according to your router ssid
const char* MQTT_SERVER = "192.168.50.30"; //Change according to your MQTT Server Settings
const char* MQTT_USER = "User_Name"; //Change according to your MQTT Server Settings
const char* MQTT_PASSWORD = "Password"; //Change according to your MQTT Server Settings

//Change IP address  as desired
//however, first three sets i.e. 192.168.50.XX should match your router address
//change XX as desired (range from 2-254)
//gateway, subnet & dns should be according to your router settings
IPAddress ip(192, 168, 50, 40);
IPAddress gateway(192, 168, 50, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 50, 1);

//add your ip address, gateway, subnet and dns in library callup
ESPMetRED client(ip, gateway, subnet, dns);

void setup() {
  //do some stuff after boot up
  
}

void loop() {
  client.keepalive();
  ESP.deepSleep(0); //Go to deep sleep for ever
}

