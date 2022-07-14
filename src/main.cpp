#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>

void setupWifi();
void setupTime();
void setupCloudIoT();
bool itsTimeToPublish();

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif // LED_BUILTIN

const int jwtExpirationSecs = 60 * 20; // 20 min (Maximum is 24H)

CloudIoTCoreDevice *deviceObject;
WiFiClientSecure *networkClientObject;
CloudIoTCoreMqtt *iotCoreObject;
MQTTClient *mqttClientObject;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  setupWifi();
  setupTime();
  setupCloudIoT();
}

void loop() {
  iotCoreObject->loop();
  delay(10);

  if (!mqttClientObject->connected()) {
    setupWifi();
    iotCoreObject->mqttConnect();
  }

  if(itsTimeToPublish()) {
    iotCoreObject->publishTelemetry("/sensors", "{\"message\": \"Hello world!\"}");
  }
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin("ssid", "password");
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
}

void setupTime() {
  configTime(0, 0, "ntp_primary", "ntp_secondary");
  Serial.print("Waiting on time sync");
  while (time(nullptr) < 1510644967) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
}

void setupCloudIoT() {
  deviceObject = new CloudIoTCoreDevice("project_id",
                                        "location",
                                        "registry_id",
                                        "device_id",
                                        "private_key_str");

  networkClientObject = new WiFiClientSecure();
  networkClientObject->setCACert("root_cert");

  mqttClientObject = new MQTTClient(512);
  mqttClientObject->setOptions(180, true, 1000); // keepAlive, cleanSession, timeout

  iotCoreObject = new CloudIoTCoreMqtt(mqttClientObject, networkClientObject, deviceObject);
  iotCoreObject->setUseLts(true);
  iotCoreObject->startMQTT();
}

bool itsTimeToPublish() {
  bool result = false;
  static unsigned long lastMillis = 0;

  if ((millis() - lastMillis) > 60000) {
    lastMillis = millis();
    result = true;
  }
  return result;
}

void messageReceived(String &topic, String &payload) {
  Serial.println("Received message on topic: " + topic);
  Serial.println("Message payload: " + payload);
}

String getJwt() {
  unsigned long currentTime = 0;
  String jwt = "";

  currentTime = time(nullptr);
  jwt = deviceObject->createJWT(currentTime, jwtExpirationSecs);
  return jwt;
}
