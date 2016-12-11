/* 
 *  This example may switch a Light ON after Sunset and turn it OFF before Sunrise
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
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

long ON_OVER_RIDE = 600000; //Over-ride time in milli seconds
long OFF_OVER_RIDE = -1200000; //Over-ride time in milli seconds
//(Negative value means x milli seconds before certain time)
unsigned long THRESHOLD = 0;

void setup() {
  pinMode(12, OUTPUT); // set pin-12 as output
  
  int state = client.ReadSPIFFS("12"); // get previously stored state of pin-12 in flash memory
  digitalWrite(12, state); // drives pin-12 to previous state

  client.OverRide(12); // search in Flash memory if pin state was over-riden manually
  //If over-ride = 0, It means pin was pulled LOW against the program schedule
  //If over-ride = 1, It means pin was pushed HIGH against the program schedule
  //If over-ride = 3, There is no over-ride associated with pin
}

void loop() {
  client.keepalive(); // system service

  if (((millis() - THRESHOLD) > 60000) || (THRESHOLD == 0)) // Perform activity every minute
  {
    THRESHOLD = millis();

    // Condition below: It will turn the relay OFF (Normally Closed) on Sunrise plus-minus over-ride time
    if((client.Time() > (client.Sunrise() + OFF_OVER_RIDE)) && (client.Time() < (client.Sunset() + ON_OVER_RIDE)) && (((state == 0) && (client.getOverride(12) == 3)) || ((state == 1) && (client.getOverride(12) == 1))))
    {
      client.setOverride(12, 3); // Release over-ride lock on certain pin (Pin_Number, Over_ride_value)
      client.WriteGPIO(12, 1); // Switch pin-12 to HIGH (Pin, State)
      Serial.println("RELAY TURNED OFF");
    }

    // Condition below: It will turn the relay ON (Normally Closed) on Sunset plus-minus over-ride time
    if(((client.Time() > (client.Sunset() + ON_OVER_RIDE)) || (client.Time() < (client.Sunrise() + OFF_OVER_RIDE))) && (((state == 1) && (client.getOverride(12) == 3)) || ((state == 0) && (client.getOverride(12) == 0))))
    {
      client.setOverride(12, 3); // Release over-ride lock on certain pin (Pin_Number, Over_ride_value)
      client.WriteGPIO(12, 0); // Switch pin-12 to LOW (Pin, State) and store it's new state in Flash memory
      Serial.println("RELAY TURNED ON");
    }
  } 
}
