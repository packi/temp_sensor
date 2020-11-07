#include "DHT.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* mqttServer = "<change me>";
const int mqttPort = 1883;

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  
  while(!Serial) { }
  
  Serial.println("setup");
  initWifi();

  client.setServer(mqttServer, mqttPort);

  dht.begin(60);
}

void initWifi() {
  Serial.println("initWifi...");
  Serial.print("Connecting");
  WiFi.begin("Freifunk", "");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWifi Connected.");
  WiFi.printDiag(Serial);
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", NULL, NULL )) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

const int capacity =JSON_OBJECT_SIZE(2);
StaticJsonDocument<capacity> doc;

void readSensor()
{
  Serial.println("Collecting temperature data.");

  float humidity = dht.readHumidity();
  delay(500);
  float temperature = dht.readTemperature();

  if (
    isnan(humidity) || 
    isnan(temperature)
  ) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C ");

  doc["temperature"] = temperature;
  doc["humidity"] = humidity;

  char buffer[128];
  serializeJson(doc, buffer);
  connectMQTT();
  if (!client.publish("/sensors/sensor1", buffer)) {
    Serial.println("Failed to send temperature");
    Serial.println(client.state());    
  }
}

void loop() {
   if (WiFi.status() != WL_CONNECTED) {
     Serial.println("Not connected...");
     initWifi();
   }
   Serial.println("Hello, World!");
   readSensor();
   delay(58000);
}
