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
    "Test",
    "test1234"
  )) {
    Serial.println("Connecting to MQTT...");
    delay(1000);
  }

  // Subscribe to Topic
  mqtt.subscribe("CENEXTGEN4/chat");
  mqtt.subscribe("ECC/7/+/+");
  Serial.println("MQTT connected");
  previousSend = millis();
}

void loop() {
  mqtt.loop();

  if (millis() - previousSend >= 1000) {
    previousSend = millis();
  // Publish to Topic
    mqtt.publish("ECC/5/01/temp", "50.1");

    // Floor 8
    mqtt.publish("ECC/8/01/temp",  "80.1");
    mqtt.publish("ECC/8/01/humid", "8.1");
    mqtt.publish("ECC/8/02/temp",  "80.2");
    mqtt.publish("ECC/8/02/humid", "8.2");

    // Floor 7
    mqtt.publish("ECC/7/01/temp",  "70.1");
    mqtt.publish("ECC/7/01/humid", "7.1");
    mqtt.publish("ECC/7/02/temp",  "70.2");
    mqtt.publish("ECC/7/02/humid", "7.2");

    mqtt.publish("CENEXTGEN4/chat", "Hi, This is P'khim");
  }
}