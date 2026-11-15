#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient network;
PubSubClient mqtt(network);

unsigned long previousSend = 0;

//callback is needed for mqtt naa Don't forget
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.write(payload, length);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  //wifi set up using 2.4 GHz for ESP32 or Cellular
  WiFi.begin("CE_NEXTGEN#4_MCI_2.4GHz", "cenextgenMCI809");

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected");

  //set mqtt broker
  mqtt.setServer("mqtt.ce-esl.com", 1883);
  mqtt.setCallback(callback);

  //login to use esl's mqtt broker
  while (!mqtt.connect(
    WiFi.macAddress().c_str(),
    "YOUR_MQTT_USER",
    "YOUR_MQTT_PASSWORD"
  )) {
    Serial.println("Connecting to MQTT...");
    delay(1000);
  }

  // Subscribe to Topic
  mqtt.subscribe("YOUR_TOPIC_HERE");

  Serial.println("MQTT connected");

  previousSend = millis();
}

void loop() {
  mqtt.loop();

  if (millis() - previousSend >= 1000) {
    previousSend = millis();
  // Publish to Topic
    mqtt.publish("YOUR_TOPIC_HERE", "YOUR_MESSAGE_HERE");
  }
}
