/*
  ESPMetRED.h - A blend of ESP8266 MQTT and Node-Red.
  Waqas Ahmed
  http://espmetred.hobbytronics.com.pk
*/

#ifndef ESPMetRED_h
#define ESPMetRED_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>

#ifdef ESP8266
#include <functional>
#define MQTT_CALLBACK_SIGN std::function<void(String)> mqttCallback
#else
#define MQTT_CALLBACK_SIGN void (*mqttCallback)(String)
#endif

#ifdef DB_DEFAULT_VALUE
#else
	#define DB_DEFAULT_VALUE 0
#endif

class ESPMetRED {
private:
	//WiFi
	unsigned long connection_watchdog = 0;
	boolean OTA_Begin = false;
	
	//Time management
	long NTP = 0;
	unsigned long _Time;
	unsigned long Time_Log = 0;
	unsigned long _Sunrise = 0;
	unsigned long _Sunset = 0;
	
	//MQTT
	MQTT_CALLBACK_SIGN;
	unsigned long ReturnACK_T = 0;
	boolean ReturnACK_C = false;
	String ReturnACK_S;
	
	//GPIOs
	byte gpio_override[] = {}; //0=off-override, 1=on-override, 2=not-applicable
	byte gpio_definition[11] = {13,12,14,16,1,3,5,4,0,2,15};
	// byte gpio_definition[] = {13,12,14,5,4,0,2};
	//byte gpio_definition[] = {13,12};

public:
	ESPMetRED();
	void joinWiFi();
	boolean WiFiScanner();
	void joinMqTT();
	void keepalive();
	
	void callback(char* topic, byte* payload, unsigned int length);
	void ReturnACK(String Identity);
	String JsonString(String mqTopic, String mqPayload);
	ESPMetRED& setCallback(MQTT_CALLBACK_SIGN);
	void PublishACK();
	void Publish(String message);
	void Publish(const char* topic, String message);
	
	void OTA();
	String networkIP();
	void systemInfo();
	
	unsigned long Timer();
	unsigned long Time();
	unsigned long Sunrise();
	unsigned long Sunset();
	
	long ReadSPIFFS(String Object);
	boolean WriteSPIFFS(String Object, long Value);
	boolean WriteGPIO(int Pin, int Value);
	
	boolean OverRide(int Pin);
	void setOverride(int Pin, int over_ride);
	int getOverride(int Pin);
};

#endif
