/*
  ESPMetRED.cpp - A blend of ESP8266 MQTT and Node-Red.
  Waqas Ahmed
  http://espmetred.hobbytronics.com.pk
*/

#include <Arduino.h>
#include <ESPMetRED.h>

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern const char* MQTT_SERVER;
extern const char* MQTT_USER;
extern const char* MQTT_PASSWORD;
extern const char* MQTT_PUBLISH_TOPIC;
extern const char* MQTT_SUBSCRIBE_TOPIC;
extern const char* CLIENT_ID;

WiFiClient _Client;
PubSubClient MQTTClient(_Client);

ESPMetRED::ESPMetRED()
{ 
	Serial.begin(115200);
	SPIFFS.begin();
	WiFi.mode(WIFI_STA);
	MQTTClient.setServer(MQTT_SERVER, 1883);
	MQTTClient.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->callback(topic, payload, length); });
	Serial.println("ESP8266 started");
	// OTA();
	NTP = ReadSPIFFS("Time");
	Serial.print("Time: ");
	Serial.println(NTP);
}

void ESPMetRED::joinWiFi()
{
	boolean access_point_check = WiFiScanner();
	if ((access_point_check) && (WiFi.status() != WL_CONNECTED))
	{
		Serial.println("Connecting to WiFi Access Point");
		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	} else if ((access_point_check) && (WiFi.status() == WL_CONNECTED))
	{
		Serial.println("WiFi connected");
	}
}

boolean ESPMetRED::WiFiScanner()
{
	Serial.println("Scanning WiFi Network");
	int n = WiFi.scanNetworks();
	delay(300);
	for (int i = 0; i < n; ++i)
	{
		if (WiFi.SSID(i) == WIFI_SSID)
		{
			Serial.println("WiFi Network Available");
			return true;
		}
		//delay(1);
	}
	Serial.println("WiFi Network NOT Available");
	return false;
}

void ESPMetRED::joinMqTT()
{
	MQTTClient.connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD);
	
	if (MQTTClient.connected()) {
		MQTTClient.subscribe(MQTT_SUBSCRIBE_TOPIC);
		MQTTClient.subscribe("ntp/OUT");
		Publish("debug", String(CLIENT_ID) + " [LOGGED IN]");
		Publish("ntp/IN", JsonString(String(MQTT_SUBSCRIBE_TOPIC), ""));
	} else {
		Serial.print("Failed to connect to mqtt server, rc=");
		Serial.print(MQTTClient.state());
		Serial.println("");
	}
}

void ESPMetRED::keepalive()
{
	if (((millis() - connection_watchdog) > 12000UL) && (WiFi.status() != WL_CONNECTED))
	{
		connection_watchdog = millis();
		joinWiFi();
	} else if (((millis() - connection_watchdog) > 12000UL) && (WiFi.status() == WL_CONNECTED) && (!MQTTClient.connected()))
	{
		joinMqTT();
	}
	if ((millis() - Time_Log) > 300000UL)
	{
		Time_Log = millis();
		WriteSPIFFS("Time", _Time);
	}
	// if ((WiFi.status() == WL_CONNECTED) && (!OTA_Begin))
	// {
		// OTA_Begin = true;
		// OTA();
	// }
	// if ((WiFi.status() == WL_CONNECTED) && (OTA_Begin))
	// {
		// ArduinoOTA.handle();
	// }
	MQTTClient.loop();
	Timer();
	
	if(_Time > 86400000L)
	{
		NTP = (0 - (millis()));
		Timer();
	}
	
	PublishACK();
	
	delay(1);
}

void ESPMetRED::callback(char* topic, byte* payload, unsigned int length)
{	
	
//	Generating String from received payload	
	String message = String();
	for (int i = 0; i < length; i++) {
		char input_char = (char)payload[i];
		message += input_char;
	}
//	Converting String to JSON Array	
	char jsonChar[100];
	message.toCharArray(jsonChar, message.length()+1);
	StaticJsonBuffer<500> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(jsonChar);
	String Topic = root["TOPIC"];
	
//	Reactions on received message
		
	if (Topic == "SYSTEM")
	{
		String Payload = root["PAYLOAD"];
		
		if (Payload == "INFO")
		{
			systemInfo();
		} 
		else if (Payload == "NTP")
		{
			unsigned long _NTP = root["NTP"];
			NTP = ((_NTP + 1000) - millis());
			_Sunrise = root["SUNRISE"];
			_Sunset = root["SUNSET"];
			ReturnACK(String(CLIENT_ID) + " [" + Payload + " OK]");
			WriteSPIFFS("Time", _Time);
		}
		else if (Payload == "OTA")
		{
			Publish("debug", String(CLIENT_ID) + " [" + Payload + " OK]");
			String Link = "/" + String(CLIENT_ID) + ".bin";
			ESPhttpUpdate.update(MQTT_SERVER, 82, Link);
		}
		else if (Payload == "REBOOT")
		{
			Publish("debug", String(CLIENT_ID) + " [" + Payload + " OK]");
			ESP.restart();
		}
	}
	
	else if (Topic == "GPIO")
	{
		int Pin = root["PIN"];
		int Value = root["VALUE"];
		setOverride(Pin, Value);
		WriteGPIO(Pin, Value);
	}
	
	else if (Topic == "OVERRIDE?")
	{
		int Pin = root["PIN"];
		Publish("debug", "OVERRIDE[" + String(gpio_override[Pin]) + "]");
	}
	else if (Topic == "GPIO?")
	{
		int Pin = root["PIN"];
		Publish("debug", "Pin[" + String(Pin) + " " + String(digitalRead(Pin)) + "]");
	}
	else if (Topic == "SPIFF")
	{
		String Object = root["OBJECT"];
		int Value = root["VALUE"];
		WriteSPIFFS(Object, Value);
		Publish("debug", "SPIFF[" + Object + " " + String(Value) + "]");
	}
	else if (Topic == "SPIFF?")
	{
		String Object = root["OBJECT"];
		int Value = ReadSPIFFS(Object);
		Publish("debug", "SPIFF?[" + Object + " " + String(Value) + "]");
	}
	else
	{
		if (mqttCallback)
		{
			mqttCallback(message);
		} else {
			String Payload;
			root.printTo(Payload);
			Publish("debug", String(CLIENT_ID) + " [COMMAND INVALID]");
		}
	}
}

void ESPMetRED::ReturnACK(String Identity)
{
	ReturnACK_C = true;
	ReturnACK_T = millis();
	ReturnACK_S = Identity;
}

String ESPMetRED::JsonString(String mqTopic, String mqPayload)
{
	StaticJsonBuffer<500> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	
	json["topic"] = mqTopic;
	json["payload"] = mqPayload;
	
	String json_object;
	json.printTo(json_object);
	return json_object;
}

ESPMetRED& ESPMetRED::setCallback(MQTT_CALLBACK_SIGN)
{
	this->mqttCallback = mqttCallback;
    return *this;
}

void ESPMetRED::PublishACK()
{
	if((ReturnACK_C) && ((millis() - ReturnACK_T) > 1))
	{
		ReturnACK_T = millis();
		ReturnACK_C = false;
		Publish("debug", ReturnACK_S);
		ReturnACK_S = "";
	}
}

void ESPMetRED::Publish(String message)
{
	MQTTClient.publish(MQTT_PUBLISH_TOPIC, message.c_str(), true);
}

void ESPMetRED::Publish(const char* topic, String message)
{
	MQTTClient.publish(topic, message.c_str());
}

String ESPMetRED::networkIP()
{
	String LocalIP;
	IPAddress ip = WiFi.localIP();
	for (int i=0; i<4; i++)
	{
		LocalIP += i  ? "." + String(ip[i]) : String(ip[i]);
		delay(10);
	}
	return LocalIP;
}

void ESPMetRED::systemInfo()
{
	Publish("debug", "ClientID [" + String(CLIENT_ID) + "]");
	Publish("debug", "MAC [" + String(WiFi.macAddress()) + "]");
	Publish("debug", "IP [" + networkIP() + "]");
	
	Publish("debug", "UPTime [" + String(millis()) + "]");
	Publish("debug", "Time [" + String(_Time) + "]");
	Publish("debug", "SunRise [" + String(_Sunrise) + "]");
	Publish("debug", "SunSet [" + String(_Sunset) + "]");
}

unsigned long ESPMetRED::Timer()
{
	_Time = (millis() + (NTP));
}

unsigned long ESPMetRED::Time()
{
	return _Time;
}

unsigned long ESPMetRED::Sunrise()
{
	return _Sunrise;
}

unsigned long ESPMetRED::Sunset()
{
	return _Sunset;
}

long ESPMetRED::ReadSPIFFS(String Object)
{
	File databaseFile = SPIFFS.open("/db.json", "r");
	if (!databaseFile) {
		Serial.println("Failed to open database file");
		return 99;
	}
	size_t size = databaseFile.size();
	if (size > 1024) {
		Serial.println("database file size is too large");
		return 99;
	}
	std::unique_ptr<char[]> buf(new char[size]);
	databaseFile.readBytes(buf.get(), size);

	StaticJsonBuffer<500> jsonBuffer;
	JsonObject& database = jsonBuffer.parseObject(buf.get());
	if (!database.success()) {
		Serial.println("Failed to parse database file");
		return 99;
	}
	
	if (database.containsKey(Object)) {
		long value = database[Object];
		Serial.println("database parsed successfully");
		return value;
	} else {
		Serial.println("database entry not present");
		return 99;
	}
}

boolean ESPMetRED::WriteSPIFFS(String Object, long Value)
{
	File databaseFile = SPIFFS.open("/db.json", "r");
	size_t size = databaseFile.size();
	std::unique_ptr<char[]> buf(new char[size]);
	databaseFile.readBytes(buf.get(), size);
	StaticJsonBuffer<500> jsonBuffer;
	JsonObject& database = jsonBuffer.parseObject(buf.get());
	
	if (database[Object] != Value) {
		database[Object] = Value;
		File databaseFile = SPIFFS.open("/db.json", "w");
		database.printTo(databaseFile);
		String serialDebug = "WriteSPIFFS [" + Object + " " + String(Value) + " OK]";
		Serial.println(serialDebug);
		return true;
	} else {
		Serial.println("Old Value resembles New Value!");
		return false;
	}
}

boolean ESPMetRED::WriteGPIO(int Pin, int Value)
{
	if((Value == 0) || (Value == 1))
	{
		int Pin_Value = digitalRead(Pin);
		if (Value == Pin_Value)
		{
			Serial.println("GPIO is already set to the Value");
			return false;
		}
		else
		{
			digitalWrite(Pin, Value);
			String printDebug = String(Pin) + " is set to " + String(Value);
			Publish("debug", printDebug);
			WriteSPIFFS(String(Pin), Value);
			Serial.println(printDebug);
			return true;
		}
	}
	else
	{
		Serial.println("Invalid Value for GPIO");
		return false;
	}
}

boolean ESPMetRED::OverRide(int Pin)
{
	String Object = "OverRide" + String(Pin);
	int state = ReadSPIFFS(Object);
	if (state == 99)
	{
		state = 3;
		delay(100);
	}
	gpio_override[Pin] = state;
}

void ESPMetRED::setOverride(int Pin, int over_ride)
{
	gpio_override[Pin] = over_ride;
	String Object = "OverRide" + String(Pin);
	WriteSPIFFS(Object, over_ride);
}

int ESPMetRED::getOverride(int Pin)
{
	return gpio_override[Pin];
}
