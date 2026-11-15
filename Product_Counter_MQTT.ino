#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define IR_SENSOR D5

// WiFi details
const char* ssid = "narzo 50A";
const char* password = "Narzo 50";

// MQTT broker
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

const char* topic = "factory/product/count";

WiFiClient espClient;
PubSubClient client(espClient);

int productCount = 0;
int sensorState = HIGH;
int lastReading = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void connectMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    String clientId = "ESP8266_ProductCounter_01";

    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT Connected!");

      client.publish(topic, "0");
    } 
    else {
      Serial.print("MQTT Failed. State: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IR_SENSOR, INPUT);

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");

  client.setServer(mqtt_server, mqtt_port);

  connectMQTT();

  Serial.println("Product Counter Started!");
}

void loop() {

  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();

  int reading = digitalRead(IR_SENSOR);

  if (reading != lastReading) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {

    if (reading != sensorState) {
      sensorState = reading;

      if (sensorState == LOW) {

        productCount++;

        Serial.print("Product Count: ");
        Serial.println(productCount);

        char message[10];
        snprintf(message, sizeof(message), "%d", productCount);

        client.publish(topic, message);
      }
    }
  }

  lastReading = reading;
}
