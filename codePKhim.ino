// ESP32 + MQTT over WebSocket Secure (wss) -> mqtt.ce-esl.com:443/mqtt
// ไลบรารี: WebSockets (links2004), MQTTPubSubClient (hideakitai)
// แก้ให้ตรงกับ API ของไลบรารีที่ติดตั้งอยู่:
//   subscribe(callback รวม)            : void(const char* topic, const char* payload, unsigned int size)
//   subscribe(topic, callback ต่อ topic): void(const char* payload, unsigned int size)

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <MQTTPubSubClient.h>

const char* WIFI_SSID     = "Khim";
const char* WIFI_PASSWORD = "Tacolato";

const char* MQTT_HOST     = "mqtt.ce-esl.com";
const int   MQTT_PORT     = 443;
const char* MQTT_PATH     = "/mqtt";
const char* MQTT_USER     = "Test";
const char* MQTT_PASSWORD = "test1234";

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

  // callback เดียวรับทุก topic (ไม่ต้องแยกฟังก์ชัน)
  mqtt.subscribe([](const char* topicC, const char* payloadC, unsigned int size) {
    String topic(topicC);
    String payload;
    payload.concat(payloadC, size);   // กัน payload ที่ไม่มี '\0' ปิดท้าย

    if (topicMatches("ECC/CENEXTGEN4/chat", topic)) {
      Serial.println("[chat] " + payload);
    }
    else if (topicMatches("ECC/7/+/+", topic)) {
      Serial.println("[floor7] " + topic + " = " + payload);
    }
    // เพิ่มได้อีก เช่น else if (topicMatches("ECC/8/+/+", topic)) { ... }
  });

  // ลงทะเบียน filter ที่ broker (ไลบรารีนี้ต้องมี callback ด้วย ใส่ตัวว่างไว้
  // เพราะข้อความจริงถูกจัดการใน callback รวมด้านบนแล้ว)
  auto noop = [](const char*, unsigned int) {};
  mqtt.subscribe("ECC/CENEXTGEN4/chat", noop);
  mqtt.subscribe("ECC/7/+/+", noop);

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

  // wss + sub-protocol "mqtt" (ยังไม่ตรวจใบรับรอง เหมาะกับทดสอบ)
  ws.beginSSL(MQTT_HOST, MQTT_PORT, MQTT_PATH, "", "mqtt");
  ws.setReconnectInterval(2000);
  mqtt.begin(ws);

  connectMqtt();
  previousSend = millis();
}

void loop() {
  mqtt.update();

  // ต่อใหม่ถ้าหลุด (ถ้าไลบรารีไม่มี isConnected() ให้ลบ 3 บรรทัดนี้)
  if (!mqtt.isConnected()) {
    connectMqtt();
  }

  if (millis() - previousSend >= 1000) {
    previousSend = millis();

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

    mqtt.publish("ECC/CENEXTGEN4/chat", "Hi, This is P'khim");
  }
}