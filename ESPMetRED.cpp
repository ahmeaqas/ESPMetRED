/*
 ESPMetRED.cpp - A Modified version of PubSubClient library to connect ESP8266 with NodeRed via MQTT.
 Ahmed
 https://espmetred.hobbytronics.com.pk
  
 Special thanks to Nick O'Leary for his PubSubClient library
 http://knolleary.net
*/

#include "ESPMetRED.h"
#include "Arduino.h"


extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern const char* MQTT_SERVER;
extern const char* MQTT_USER;
extern const char* MQTT_PASSWORD;
extern const char* MQTT_PUBLISH_TOPIC;
extern const char* MQTT_SUBSCRIBE_TOPIC;
extern const char* CLIENT_ID;

WiFiClient _Client;

ESPMetRED::ESPMetRED() {
    this->_state = MQTT_DISCONNECTED;
    setClient(_Client);
    this->stream = NULL;
	

	Serial.begin(115200);
	SPIFFS.begin();
	WiFi.mode(WIFI_STA);
	setServer(MQTT_SERVER, 1883);
	setCallback([this] (char* topic, byte* payload, unsigned int length) { this->mqttin(topic, payload, length); });
	Serial.println("ESP8266 started");
	// OTA();
	NTP = ReadSPIFFS("Time");
	Serial.print("Time: ");
	Serial.println(NTP);
}

ESPMetRED::ESPMetRED(IPAddress addr, uint16_t port, Client& client) {
    this->_state = MQTT_DISCONNECTED;
    setServer(addr, port);
    setClient(client);
    this->stream = NULL;
}
ESPMetRED::ESPMetRED(IPAddress addr, uint16_t port, Client& client, Stream& stream) {
    this->_state = MQTT_DISCONNECTED;
    setServer(addr,port);
    setClient(client);
    setStream(stream);
}
ESPMetRED::ESPMetRED(IPAddress addr, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) {
    this->_state = MQTT_DISCONNECTED;
    setServer(addr, port);
    setCallback(callback);
    setClient(client);
    this->stream = NULL;
}
ESPMetRED::ESPMetRED(IPAddress addr, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) {
    this->_state = MQTT_DISCONNECTED;
    setServer(addr,port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

ESPMetRED::ESPMetRED(uint8_t *ip, uint16_t port, Client& client) {
    this->_state = MQTT_DISCONNECTED;
    setServer(ip, port);
    setClient(client);
    this->stream = NULL;
}
ESPMetRED::ESPMetRED(uint8_t *ip, uint16_t port, Client& client, Stream& stream) {
    this->_state = MQTT_DISCONNECTED;
    setServer(ip,port);
    setClient(client);
    setStream(stream);
}
ESPMetRED::ESPMetRED(uint8_t *ip, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) {
    this->_state = MQTT_DISCONNECTED;
    setServer(ip, port);
    setCallback(callback);
    setClient(client);
    this->stream = NULL;
}
ESPMetRED::ESPMetRED(uint8_t *ip, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) {
    this->_state = MQTT_DISCONNECTED;
    setServer(ip,port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

ESPMetRED::ESPMetRED(const char* domain, uint16_t port, Client& client) {
    this->_state = MQTT_DISCONNECTED;
    setServer(domain,port);
    setClient(client);
    this->stream = NULL;
}
ESPMetRED::ESPMetRED(const char* domain, uint16_t port, Client& client, Stream& stream) {
    this->_state = MQTT_DISCONNECTED;
    setServer(domain,port);
    setClient(client);
    setStream(stream);
}
ESPMetRED::ESPMetRED(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client) {
    this->_state = MQTT_DISCONNECTED;
    setServer(domain,port);
    setCallback(callback);
    setClient(client);
    this->stream = NULL;
}
ESPMetRED::ESPMetRED(const char* domain, uint16_t port, MQTT_CALLBACK_SIGNATURE, Client& client, Stream& stream) {
    this->_state = MQTT_DISCONNECTED;
    setServer(domain,port);
    setCallback(callback);
    setClient(client);
    setStream(stream);
}

boolean ESPMetRED::connect(const char *id) {
    return connect(id,NULL,NULL,0,0,0,0);
}

boolean ESPMetRED::connect(const char *id, const char *user, const char *pass) {
    return connect(id,user,pass,0,0,0,0);
}

boolean ESPMetRED::connect(const char *id, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage) {
    return connect(id,NULL,NULL,willTopic,willQos,willRetain,willMessage);
}

boolean ESPMetRED::connect(const char *id, const char *user, const char *pass, const char* willTopic, uint8_t willQos, boolean willRetain, const char* willMessage) {
    if (!connected()) {
        int result = 0;

        if (domain != NULL) {
            result = _client->connect(this->domain, this->port);
        } else {
            result = _client->connect(this->ip, this->port);
        }
        if (result) {
            nextMsgId = 1;
            // Leave room in the buffer for header and variable length field
            uint16_t length = 5;
            unsigned int j;

#if MQTT_VERSION == MQTT_VERSION_3_1
            uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p', MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
            uint8_t d[7] = {0x00,0x04,'M','Q','T','T',MQTT_VERSION};
#define MQTT_HEADER_VERSION_LENGTH 7
#endif
            for (j = 0;j<MQTT_HEADER_VERSION_LENGTH;j++) {
                buffer[length++] = d[j];
            }

            uint8_t v;
            if (willTopic) {
                v = 0x06|(willQos<<3)|(willRetain<<5);
            } else {
                v = 0x02;
            }

            if(user != NULL) {
                v = v|0x80;

                if(pass != NULL) {
                    v = v|(0x80>>1);
                }
            }

            buffer[length++] = v;

            buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
            buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
            length = writeString(id,buffer,length);
            if (willTopic) {
                length = writeString(willTopic,buffer,length);
                length = writeString(willMessage,buffer,length);
            }

            if(user != NULL) {
                length = writeString(user,buffer,length);
                if(pass != NULL) {
                    length = writeString(pass,buffer,length);
                }
            }

            write(MQTTCONNECT,buffer,length-5);

            lastInActivity = lastOutActivity = millis();

            while (!_client->available()) {
                unsigned long t = millis();
                if (t-lastInActivity >= ((int32_t) MQTT_SOCKET_TIMEOUT*1000UL)) {
                    _state = MQTT_CONNECTION_TIMEOUT;
                    _client->stop();
                    return false;
                }
            }
            uint8_t llen;
            uint16_t len = readPacket(&llen);

            if (len == 4) {
                if (buffer[3] == 0) {
                    lastInActivity = millis();
                    pingOutstanding = false;
                    _state = MQTT_CONNECTED;
                    return true;
                } else {
                    _state = buffer[3];
                }
            }
            _client->stop();
        } else {
            _state = MQTT_CONNECT_FAILED;
        }
        return false;
    }
    return true;
}

// reads a byte into result
boolean ESPMetRED::readByte(uint8_t * result) {
   uint32_t previousMillis = millis();
   while(!_client->available()) {
     uint32_t currentMillis = millis();
     if(currentMillis - previousMillis >= ((int32_t) MQTT_SOCKET_TIMEOUT * 1000)){
       return false;
     }
   }
   *result = _client->read();
   return true;
}

// reads a byte into result[*index] and increments index
boolean ESPMetRED::readByte(uint8_t * result, uint16_t * index){
  uint16_t current_index = *index;
  uint8_t * write_address = &(result[current_index]);
  if(readByte(write_address)){
    *index = current_index + 1;
    return true;
  }
  return false;
}

uint16_t ESPMetRED::readPacket(uint8_t* lengthLength) {
    uint16_t len = 0;
    if(!readByte(buffer, &len)) return 0;
    bool isPublish = (buffer[0]&0xF0) == MQTTPUBLISH;
    uint32_t multiplier = 1;
    uint16_t length = 0;
    uint8_t digit = 0;
    uint16_t skip = 0;
    uint8_t start = 0;

    do {
        if(!readByte(&digit)) return 0;
        buffer[len++] = digit;
        length += (digit & 127) * multiplier;
        multiplier *= 128;
    } while ((digit & 128) != 0);
    *lengthLength = len-1;

    if (isPublish) {
        // Read in topic length to calculate bytes to skip over for Stream writing
        if(!readByte(buffer, &len)) return 0;
        if(!readByte(buffer, &len)) return 0;
        skip = (buffer[*lengthLength+1]<<8)+buffer[*lengthLength+2];
        start = 2;
        if (buffer[0]&MQTTQOS1) {
            // skip message id
            skip += 2;
        }
    }

    for (uint16_t i = start;i<length;i++) {
        if(!readByte(&digit)) return 0;
        if (this->stream) {
            if (isPublish && len-*lengthLength-2>skip) {
                this->stream->write(digit);
            }
        }
        if (len < MQTT_MAX_PACKET_SIZE) {
            buffer[len] = digit;
        }
        len++;
    }

    if (!this->stream && len > MQTT_MAX_PACKET_SIZE) {
        len = 0; // This will cause the packet to be ignored.
    }

    return len;
}

boolean ESPMetRED::loop() {
    if (connected()) {
        unsigned long t = millis();
        if ((t - lastInActivity > MQTT_KEEPALIVE*1000UL) || (t - lastOutActivity > MQTT_KEEPALIVE*1000UL)) {
            if (pingOutstanding) {
                this->_state = MQTT_CONNECTION_TIMEOUT;
                _client->stop();
                return false;
            } else {
                buffer[0] = MQTTPINGREQ;
                buffer[1] = 0;
                _client->write(buffer,2);
                lastOutActivity = t;
                lastInActivity = t;
                pingOutstanding = true;
            }
        }
        if (_client->available()) {
            uint8_t llen;
            uint16_t len = readPacket(&llen);
            uint16_t msgId = 0;
            uint8_t *payload;
            if (len > 0) {
                lastInActivity = t;
                uint8_t type = buffer[0]&0xF0;
                if (type == MQTTPUBLISH) {
                    if (callback) {
                        uint16_t tl = (buffer[llen+1]<<8)+buffer[llen+2];
                        char topic[tl+1];
                        for (uint16_t i=0;i<tl;i++) {
                            topic[i] = buffer[llen+3+i];
                        }
                        topic[tl] = 0;
                        // msgId only present for QOS>0
                        if ((buffer[0]&0x06) == MQTTQOS1) {
                            msgId = (buffer[llen+3+tl]<<8)+buffer[llen+3+tl+1];
                            payload = buffer+llen+3+tl+2;
                            callback(topic,payload,len-llen-3-tl-2);

                            buffer[0] = MQTTPUBACK;
                            buffer[1] = 2;
                            buffer[2] = (msgId >> 8);
                            buffer[3] = (msgId & 0xFF);
                            _client->write(buffer,4);
                            lastOutActivity = t;

                        } else {
                            payload = buffer+llen+3+tl;
                            callback(topic,payload,len-llen-3-tl);
                        }
                    }
                } else if (type == MQTTPINGREQ) {
                    buffer[0] = MQTTPINGRESP;
                    buffer[1] = 0;
                    _client->write(buffer,2);
                } else if (type == MQTTPINGRESP) {
                    pingOutstanding = false;
                }
            }
        }
        return true;
    }
    return false;
}

boolean ESPMetRED::publish(const char* topic, const char* payload) {
    return publish(topic,(const uint8_t*)payload,strlen(payload),false);
}

boolean ESPMetRED::publish(const char* topic, const char* payload, boolean retained) {
    return publish(topic,(const uint8_t*)payload,strlen(payload),retained);
}

boolean ESPMetRED::publish(const char* topic, const uint8_t* payload, unsigned int plength) {
    return publish(topic, payload, plength, false);
}

boolean ESPMetRED::publish(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained) {
    if (connected()) {
        if (MQTT_MAX_PACKET_SIZE < 5 + 2+strlen(topic) + plength) {
            // Too long
            return false;
        }
        // Leave room in the buffer for header and variable length field
        uint16_t length = 5;
        length = writeString(topic,buffer,length);
        uint16_t i;
        for (i=0;i<plength;i++) {
            buffer[length++] = payload[i];
        }
        uint8_t header = MQTTPUBLISH;
        if (retained) {
            header |= 1;
        }
        return write(header,buffer,length-5);
    }
    return false;
}

boolean ESPMetRED::publish_P(const char* topic, const uint8_t* payload, unsigned int plength, boolean retained) {
    uint8_t llen = 0;
    uint8_t digit;
    unsigned int rc = 0;
    uint16_t tlen;
    unsigned int pos = 0;
    unsigned int i;
    uint8_t header;
    unsigned int len;

    if (!connected()) {
        return false;
    }

    tlen = strlen(topic);

    header = MQTTPUBLISH;
    if (retained) {
        header |= 1;
    }
    buffer[pos++] = header;
    len = plength + 2 + tlen;
    do {
        digit = len % 128;
        len = len / 128;
        if (len > 0) {
            digit |= 0x80;
        }
        buffer[pos++] = digit;
        llen++;
    } while(len>0);

    pos = writeString(topic,buffer,pos);

    rc += _client->write(buffer,pos);

    for (i=0;i<plength;i++) {
        rc += _client->write((char)pgm_read_byte_near(payload + i));
    }

    lastOutActivity = millis();

    return rc == tlen + 4 + plength;
}

boolean ESPMetRED::write(uint8_t header, uint8_t* buf, uint16_t length) {
    uint8_t lenBuf[4];
    uint8_t llen = 0;
    uint8_t digit;
    uint8_t pos = 0;
    uint16_t rc;
    uint16_t len = length;
    do {
        digit = len % 128;
        len = len / 128;
        if (len > 0) {
            digit |= 0x80;
        }
        lenBuf[pos++] = digit;
        llen++;
    } while(len>0);

    buf[4-llen] = header;
    for (int i=0;i<llen;i++) {
        buf[5-llen+i] = lenBuf[i];
    }

#ifdef MQTT_MAX_TRANSFER_SIZE
    uint8_t* writeBuf = buf+(4-llen);
    uint16_t bytesRemaining = length+1+llen;  //Match the length type
    uint8_t bytesToWrite;
    boolean result = true;
    while((bytesRemaining > 0) && result) {
        bytesToWrite = (bytesRemaining > MQTT_MAX_TRANSFER_SIZE)?MQTT_MAX_TRANSFER_SIZE:bytesRemaining;
        rc = _client->write(writeBuf,bytesToWrite);
        result = (rc == bytesToWrite);
        bytesRemaining -= rc;
        writeBuf += rc;
    }
    return result;
#else
    rc = _client->write(buf+(4-llen),length+1+llen);
    lastOutActivity = millis();
    return (rc == 1+llen+length);
#endif
}

boolean ESPMetRED::subscribe(const char* topic) {
    return subscribe(topic, 0);
}

boolean ESPMetRED::subscribe(const char* topic, uint8_t qos) {
    if (qos < 0 || qos > 1) {
        return false;
    }
    if (MQTT_MAX_PACKET_SIZE < 9 + strlen(topic)) {
        // Too long
        return false;
    }
    if (connected()) {
        // Leave room in the buffer for header and variable length field
        uint16_t length = 5;
        nextMsgId++;
        if (nextMsgId == 0) {
            nextMsgId = 1;
        }
        buffer[length++] = (nextMsgId >> 8);
        buffer[length++] = (nextMsgId & 0xFF);
        length = writeString((char*)topic, buffer,length);
        buffer[length++] = qos;
        return write(MQTTSUBSCRIBE|MQTTQOS1,buffer,length-5);
    }
    return false;
}

boolean ESPMetRED::unsubscribe(const char* topic) {
    if (MQTT_MAX_PACKET_SIZE < 9 + strlen(topic)) {
        // Too long
        return false;
    }
    if (connected()) {
        uint16_t length = 5;
        nextMsgId++;
        if (nextMsgId == 0) {
            nextMsgId = 1;
        }
        buffer[length++] = (nextMsgId >> 8);
        buffer[length++] = (nextMsgId & 0xFF);
        length = writeString(topic, buffer,length);
        return write(MQTTUNSUBSCRIBE|MQTTQOS1,buffer,length-5);
    }
    return false;
}

void ESPMetRED::disconnect() {
    buffer[0] = MQTTDISCONNECT;
    buffer[1] = 0;
    _client->write(buffer,2);
    _state = MQTT_DISCONNECTED;
    _client->stop();
    lastInActivity = lastOutActivity = millis();
}

uint16_t ESPMetRED::writeString(const char* string, uint8_t* buf, uint16_t pos) {
    const char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos-i-2] = (i >> 8);
    buf[pos-i-1] = (i & 0xFF);
    return pos;
}


boolean ESPMetRED::connected() {
    boolean rc;
    if (_client == NULL ) {
        rc = false;
    } else {
        rc = (int)_client->connected();
        if (!rc) {
            if (this->_state == MQTT_CONNECTED) {
                this->_state = MQTT_CONNECTION_LOST;
                _client->flush();
                _client->stop();
            }
        }
    }
    return rc;
}

ESPMetRED& ESPMetRED::setServer(uint8_t * ip, uint16_t port) {
    IPAddress addr(ip[0],ip[1],ip[2],ip[3]);
    return setServer(addr,port);
}

ESPMetRED& ESPMetRED::setServer(IPAddress ip, uint16_t port) {
    this->ip = ip;
    this->port = port;
    this->domain = NULL;
    return *this;
}

ESPMetRED& ESPMetRED::setServer(const char * domain, uint16_t port) {
    this->domain = domain;
    this->port = port;
    return *this;
}

ESPMetRED& ESPMetRED::setCallback(MQTT_CALLBACK_SIGNATURE) {
    this->callback = callback;
    return *this;
}

ESPMetRED& ESPMetRED::setClient(Client& client){
    this->_client = &client;
    return *this;
}

ESPMetRED& ESPMetRED::setStream(Stream& stream){
    this->stream = &stream;
    return *this;
}

int ESPMetRED::state() {
    return this->_state;
}

//---------------------------------------------------------------

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
	connect(CLIENT_ID, MQTT_USER, MQTT_PASSWORD);
	
	if (connected()) {
		subscribe(MQTT_SUBSCRIBE_TOPIC);
		subscribe("ntp/OUT");
		Publish("debug", String(CLIENT_ID) + " [LOGGED IN]");
		Publish("ntp/IN", JsonString(String(MQTT_SUBSCRIBE_TOPIC), ""));
	} else {
		Serial.print("Failed to connect to mqtt server, rc=");
		Serial.print(state());
		Serial.println("");
	}
}

void ESPMetRED::keepalive()
{
	if (((millis() - connection_watchdog) > 12000UL) && (WiFi.status() != WL_CONNECTED))
	{
		connection_watchdog = millis();
		joinWiFi();
	} else if (((millis() - connection_watchdog) > 12000UL) && (WiFi.status() == WL_CONNECTED) && (!connected()))
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
	loop();
	Timer();
	
	if(_Time > 86400000L)
	{
		NTP = (0 - (millis()));
		Timer();
	}
	
	PublishACK();
	
	delay(1);
}

int ESPMetRED::NetStat()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		return 1;
	}
	if ((WiFi.status() == WL_CONNECTED) && (!connected()))
	{
		return 2;
	}
	if ((WiFi.status() == WL_CONNECTED) && (connected()))
	{
		return 3;
	}
}

void ESPMetRED::mqttin(char* topic, byte* payload, unsigned int length)
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

ESPMetRED& ESPMetRED::setcallback(MQTT_CALLBACK_SIGN)
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
	publish(MQTT_PUBLISH_TOPIC, message.c_str(), true);
}

void ESPMetRED::Publish(const char* topic, String message)
{
	publish(topic, message.c_str());
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
			String printDebug = String(CLIENT_ID) + " [GPIO " + String(Pin) + " " + String(Value) + "]";
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
