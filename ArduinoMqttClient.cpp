#include <WiFiS3.h>
#include <ArduinoMqttClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char* topic_publish = "arduino/sensor_data";
const char* topic_subscribe = "arduino/commands";

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

int analogPin = A0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  mqttClient.setId("ArduinoR4Client");
  mqttClient.setUsernamePassword("", "");

  mqttClient.onMessage(onMqttMessage);

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(mqtt_broker);
  if (!mqttClient.connect(mqtt_broker, mqtt_port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");

  mqttClient.subscribe(topic_subscribe);
  Serial.print("Subscribed to topic: ");
  Serial.println(topic_subscribe);
}

void loop() {
  mqttClient.poll();

  static unsigned long lastPublish = 0;
  if (millis() - lastPublish > 10000) {
    lastPublish = millis();

    int sensorValue = analogRead(analogPin);
    String payload = String(sensorValue);

    Serial.print("Publishing message to topic ");
    Serial.print(topic_publish);
    Serial.print(": ");
    Serial.println(payload);

    mqttClient.beginMessage(topic_publish);
    mqttClient.print(payload);
    mqttClient.endMessage();
  }
}

void onMqttMessage(int messageSize) {
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  String message;
  while (mqttClient.available()) {
    char c = (char)mqttClient.read();
    message += c;
    Serial.print(c);
  }
  Serial.println();

  if (message == "ON") {
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (message == "OFF") {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
