#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient network;
PubSubClient mqtt(network);
unsigned long previousSend = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.write(payload, length);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  WiFi.begin("YOUR_WIFI_NAME", "YOUR_WIFI_PASSWORD");
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");

  mqtt.setServer("YOUR_BROKER_ADDRESS", 1883);
  mqtt.setCallback(callback);

  while (!mqtt.connect(WiFi.macAddress().c_str())) {
    Serial.println("Connecting to MQTT...");
    delay(1000);
  }

  mqtt.subscribe("YOUR_TOPIC_HERE");
  Serial.println("MQTT connected");
  previousSend = millis();
}

void loop() {
  mqtt.loop();

  if (millis() - previousSend >= 1000) {
    previousSend = millis();
    mqtt.publish("YOUR_TOPIC_HERE", "YOUR_MESSAGE_HERE");
  }
}