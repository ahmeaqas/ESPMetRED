/*
 ESPMetRED.cpp - A Modified version of PubSubClient library to connect ESP8266 with NodeRed via MQTT.
 Ahmed
 https://espmetred.hobbytronics.com.pk
  
 Special thanks to Nick O'Leary for his PubSubClient library
 http://knolleary.net
*/

#ifndef ESPMetRED_h
#define ESPMetRED_h

#include <Arduino.h>
#include "IPAddress.h"
#include "Client.h"
#include "Stream.h"
//--------------------------------
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>
//--------------------------------

#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4

// MQTT_VERSION : Pick the version
//#define MQTT_VERSION MQTT_VERSION_3_1
#define MQTT_VERSION MQTT_VERSION_3_1_1

// MQTT_MAX_PACKET_SIZE : Maximum packet size
#define MQTT_MAX_PACKET_SIZE 128

// MQTT_KEEPALIVE : keepAlive interval in Seconds
#define MQTT_KEEPALIVE 15

// MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#define MQTT_SOCKET_TIMEOUT 15

// MQTT_MAX_TRANSFER_SIZE : limit how much data is passed to the network client
//  in each write call. Needed for the Arduino Wifi Shield. Leave undefined to
//  pass the entire MQTT packet in each write call.
//#define MQTT_MAX_TRANSFER_SIZE 80

// Possible values for client.state()
#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

#ifdef ESP8266
#include <functional>
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, uint32_t)> callback
#define MQTT_CALLBACK_SIGN std::function<void(String)> mqttCallback
#else
#define MQTT_CALLBACK_SIGNATURE void (*callback)(char*, uint8_t*, uint32_t)
#define MQTT_CALLBACK_SIGN void (*mqttCallback)(String)
#endif

class ESPMetRED {
private:
   Client* _client;
   uint8_t buffer[MQTT_MAX_PACKET_SIZE];
   uint16_t nextMsgId;
   unsigned long lastOutActivity;
   unsigned long lastInActivity;
   bool pingOutstanding;
   MQTT_CALLBACK_SIGNATURE;
   uint16_t readPacket(uint8_t*);
   boolean readByte(uint8_t * result);
   boolean readByte(uint8_t * result, uint16_t * index);
   boolean write(uint8_t header, uint8_t* buf, uint16_t length);
   uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos);
   IPAddress ip;
   const char* domain;
   uint16_t port;
   Stream* stream;
   int _state;
   

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

public:
   ESPMetRED();
   ESPMetRED(Client& client);
   ESPMetRED(IPAddress, uint16_t, Client& client);
   ESPMetRED(IPAddress, uint16_t, Client& client, Stream&);
   ESPMetRED(IPAddress, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client);
   ESPMetRED(IPAddress, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client, Stream&);
   ESPMetRED(uint8_t *, uint16_t, Client& client);
   ESPMetRED(uint8_t *, uint16_t, Client& client, Stream&);
   ESPMetRED(uint8_t *, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client);
   ESPMetRED(uint8_t *, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client, Stream&);
   ESPMetRED(const char*, uint16_t, Client& client);
   ESPMetRED(const char*, uint16_t, Client& client, Stream&);
   ESPMetRED(const char*, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client);
   ESPMetRED(const char*, uint16_t, MQTT_CALLBACK_SIGNATURE,Client& client, Stream&);

   ESPMetRED& setServer(IPAddress ip, uint16_t port);
   ESPMetRED& setServer(uint8_t * ip, uint16_t port);
   ESPMetRED& setServer(const char * domain, uint16_t port);
   ESPMetRED& setCallback(MQTT_CALLBACK_SIGNATURE);
   ESPMetRED& setClient(Client& client);
   ESPMetRED& setStream(Stream& stream);

   boolean connect(const char* id);
   boolean connect(const char* id, const char* user, const char* pass);
   boolean connect(const char* id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
   boolean connect(const char* id, const char* user, const char* pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage);
   void disconnect();
   boolean publish(const char* topic, const char* payload);
   boolean publish(const char* topic, const char* payload, boolean retained);
   boolean publish(const char* topic, const uint8_t * payload, unsigned int plength);
   boolean publish(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained);
   boolean publish_P(const char* topic, const uint8_t * payload, unsigned int plength, boolean retained);
   boolean subscribe(const char* topic);
   boolean subscribe(const char* topic, uint8_t qos);
   boolean unsubscribe(const char* topic);
   boolean loop();
   boolean connected();
   int state();
   
	void joinWiFi();
	boolean WiFiScanner();
	void joinMqTT();
	void keepalive();
	int NetStat();
	
	void mqttin(char* topic, byte* payload, unsigned int length);
	void ReturnACK(String Identity);
	String JsonString(String mqTopic, String mqPayload);
	ESPMetRED& setcallback(MQTT_CALLBACK_SIGN);
	void PublishACK();
	void Publish(String message);
	void Publish(const char* topic, String message);

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
