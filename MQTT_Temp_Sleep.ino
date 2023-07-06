#include <WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 17

#define uS_TO_S_FACTOR 1000000        /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 900              /* Time ESP32 will go to sleep for 15 minutes (in seconds) */
#define TIME_TO_SLEEP_ERROR 3600       /* Time to sleep in case of error (1 hour) */

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;
WiFiClient espClient;
PubSubClient client(espClient);

float tempC; //temp as Float
char msg[7]; //temp as String

void setup() {
  Serial.begin(9600); 
  pinMode(17, INPUT_PULLUP); //no external resistor is needed
  setup_wifi();              //Connect to Wifi network
  client.setServer("Sperling.haus", 1883);    // Configure MQTT connection, change port if needed.

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  sensors.setResolution(insideThermometer, 9);
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  if (!client.connected()) {
    reconnect();
  }

  tempC = sensors.getTempC(insideThermometer);

  if(tempC == DEVICE_DISCONNECTED_C) 
  {
    Serial.println("Error: Could not read temperature data");
  }else{
    Serial.print("Temp C: ");
    Serial.println(tempC);
  }

  dtostrf(tempC,7,2,msg);
  client.publish("Albert", msg, true);   // Publish temperature on broker/temp1
  Serial.println("Temperature sent to MQTT.");
  delay(100); //some delay is needed for the mqtt server to accept the message
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //go to sleep
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  esp_deep_sleep_start();
}

//Setup connection to wifi
void setup_wifi() {

  delay(20);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SECRET_SSID);
  WiFi.begin(SECRET_SSID, SECRET_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

 Serial.print("=> ESP32 new IP address is: ");
 Serial.println(WiFi.localIP());
}

//Reconnect to wifi if connection is lost
void reconnect() {

  while (!client.connected()) {
    Serial.print("Connecting to MQTT broker ...");
    if (client.connect("Albert", MQTT_USER, MQTT_PASS)) {
      Serial.println("OK");
    } else {
      Serial.print("[Error] Not connected: ");
      Serial.print(client.state());
      Serial.println("Wait 5 seconds before retry.");
      delay(5000);
    }
  }
}

void loop() { 
}