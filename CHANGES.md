0.6.3
   * Initialize WiFi with Static IP Address (see deep_sleep example)
   * Connect to WiFi netwrok within 2 seconds after waking from Deep sleep
   * No WiFi connections problem associated with static ip after waking from deep sleep
   * Change OTA update port by defining it's value e.g. #define OTA_PORT XX where XX is your desired port
   
0.6.2
   * Revert back PubSubClient library merger

0.6.1
   * Compatibility with Arduino IDE Library Manager
   
0.6.0
   * ESPMetRed Library merged in PubSubClient library
   * No need to include PubSubClient library in Arduino IDE Sketch
   * Custom callback function changed to 
   	* setcallback(callback)
	* Example: client.setcallback(callback)

0.5.1
   * Added support for gpio over-ride (An over-ride from regular schedule)

0.5
   * Add function to Encode data to JSON String
   * Implement Timer along with Sunrise and Sunset
   * Change Over the Air Update (OTA) to Webserver
	 due to problems in OTA via Arduino IDE
   
0.4
   * Retrieve data from Flash memory
   * Implement Over the Air Update (OTA) using Arduino IDE

0.3
   * Add method to retrieve system information
   * Store information in Flash memory
   * Store GPIO state in Flash memory right after changing it's state

0.2
   * Add support for MQTT callback function
   * Decoding JSON string in MQTT payload

0.1
   * Auto connect with WiFi and MQTT server in case of failure 
