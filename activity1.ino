#include <WiFi.h>
#include <WebSocketsClient.h>
#include <MQTTPubSubClient.h>

const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const char* MQTT_HOST     = "mqtt.ce-esl.com";
const int   MQTT_PORT     = 443;
const char* MQTT_PATH     = "/mqtt";
const char* MQTT_USER     = "YOUR_MQTT_USER";
const char* MQTT_PASSWORD = "YOUR_MQTT_PASSWORD";

WebSocketsClient ws;
MQTTPubSubClient mqtt;
unsigned long previousSend = 0;

// เทียบ topic กับ filter (รองรับ + และ #)
bool topicMatches(const String& filter, const String& topic) {
  int f = 0, t = 0;
  while (true) {
    int fe = filter.indexOf('/', f); if (fe < 0) fe = filter.length();
    int te = topic.indexOf('/', t);  if (te < 0) te = topic.length();
    String fs = filter.substring(f, fe);
    String ts = topic.substring(t, te);

    if (fs == "#") return true;
    if (fs != "+" && fs != ts) return false;

    bool fLast = (fe >= (int)filter.length());
    bool tLast = (te >= (int)topic.length());
    if (tLast && !fLast) return filter.substring(fe + 1) == "#";
    if (fLast || tLast) return fLast && tLast;
    f = fe + 1;
    t = te + 1;
  }
}

void connectMqtt() {
  while (!mqtt.connect(WiFi.macAddress().c_str(), MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("Connecting to MQTT...");
    delay(1000);
  }

  mqtt.subscribe([](const char* topicC, const char* payloadC, unsigned int size) {
    String topic(topicC);
    String payload;
    payload.concat(payloadC, size);

    if (topicMatches("ECC/CENEXTGEN4/chat", topic)) {
      Serial.println("[chat] " + payload);
    }
    else if (topicMatches("ECC/7/+/+", topic)) {
      Serial.println("[floor7] " + topic + " = " + payload);
    }
  });

  auto noop = [](const char*, unsigned int) {};
  mqtt.subscribe("ECC/8/01/temp",noop);// subscribe to temp in 801, floor 8, ECC
  mqtt.subscribe("ECC/8/+/temp",noop);// subscribe to temp in every classroom, floor 8,ECC
  mqtt.subscribe("ECC/8/+/+",noop);// subscribe to every type of data in every classroom, floor 8,ECC
  mqtt.subscribe("ECC/+/+/+",noop);// subscribe to every type of data in every classroom, every floor,ECC ( ECC/# can also be used )


  Serial.println("MQTT connected");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");

  ws.beginSSL(MQTT_HOST, MQTT_PORT, MQTT_PATH, "", "mqtt");
  ws.setReconnectInterval(2000);
  mqtt.begin(ws);

  connectMqtt();
  previousSend = millis();
}

void loop() {
  mqtt.update();

  if (!mqtt.isConnected()) {
    connectMqtt();
  }

  if (millis() - previousSend >= 1000) {
    previousSend = millis();
    mqtt.publish("YOUR_TOPIC_HERE", "YOUR_MESSAGE_HERE");
  }
}