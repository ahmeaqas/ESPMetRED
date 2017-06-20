#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <ESPMetRED.h>
#include <math.h>
#include <DHT.h>

const char* MQTT_PUBLISH_TOPIC = "ESPMetRED/OUT";
const char* MQTT_SUBSCRIBE_TOPIC = "ESPMetRED/IN";
const char* CLIENT_ID = "ESPMetRED";
const char* WIFI_SSID = "Ahmed";
const char* WIFI_PASSWORD = "Password";
const char* MQTT_SERVER = "192.168.0.30";
const char* MQTT_USER = "User_Name";
const char* MQTT_PASSWORD = "Password"; 

ESPMetRED client;

#define DHTTYPE DHT22
#define DHTPIN  13  // Pin to which DHT-22 is attached
DHT dht(DHTPIN, DHTTYPE);

unsigned long read_dht22 = 0;
int Humidity = 0;
int Temperature = 0;
int HeatIndex = 0;

void setup() {
  dht.begin();
}

void loop() {
  client.keepalive();
  if ((millis() - read_dht22) > 2000)
  {
    read_dht22 = millis();
    dhtProbe();
  }
  
}

void dhtProbe()
{
  float h = dht.readHumidity();
  int _h = round(h);
  float t = dht.readTemperature();
  int _t = round(t);
  float hic = dht.computeHeatIndex(t, h, false);
  int _hic = round(hic);

  String sensor_data = "Humidity[" + String(h) + "%] Temperature[" + String(t) + "°C] Heat Index[" + String(hic) + "°C]";
  Serial.println(sensor_data);
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    if ((_h != Humidity) || (_t != Temperature) || (_hic != HeatIndex))
    {
      Humidity = _h;
      Temperature = _t;
      HeatIndex = _hic;

      StaticJsonBuffer<500> jsonBuffer;
      JsonObject& json = jsonBuffer.createObject();
      
      json["HUMIDITY"] = Humidity;
      json["TEMPERATURE"] = Temperature;
      json["HEATINDEX"] = HeatIndex;
      
      String json_object;
      json.printTo(json_object);
      client.Publish(json_object);
      
      Serial.println("DHT-22 sensor data published to server");
    }
  }
}
