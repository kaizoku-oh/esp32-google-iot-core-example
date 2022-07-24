#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <CloudIoTCore.h>
#include <CloudIoTCoreMqtt.h>

#include "iot_core_config.h"

void setupWifi();
void setupTime();
void setupCloudIoT();
bool itsTimeToPublish();

#define SERIAL_SPEED             115200
#define WIFI_SSID                "REPLACE_WITH_YOUR_SSID"
#define WIFI_PASSWORD            "REPLACE_WITH_YOUR_PASSWORD"
#define PRIMARY_NTP_SERVER       "pool.ntp.org"
#define SECONDARY_NTP_SERVER     "time.nist.gov"

CloudIoTCoreDevice *deviceObject;
WiFiClientSecure *networkClientObject;
CloudIoTCoreMqtt *iotCoreObject;
MQTTClient *mqttClientObject;

void setup() {
  Serial.begin(SERIAL_SPEED);

  setupWifi();
  setupTime();
  setupCloudIoT();
  iotCoreObject->publishState("{\"message\": \"Device started successfully!\"}");
}

void loop() {
  iotCoreObject->loop();
  delay(10);

  if (!mqttClientObject->connected()) {
    setupWifi();
    iotCoreObject->mqttConnect();
  }

  if (itsTimeToPublish()) {
    Serial.println("It's time to publish telemetry");
    iotCoreObject->publishTelemetry("/sensors", "{\"message\": \"Hello world!\"}");
  }
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
}

void setupTime() {
  struct tm timeInfo;

  configTime(0, 0, PRIMARY_NTP_SERVER, SECONDARY_NTP_SERVER);
  Serial.print("Waiting on time sync");
  while (time(nullptr) < 1510644967) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  if (getLocalTime(&timeInfo)) {
    Serial.println(&timeInfo, "Today is: %A, %B %d %Y %H:%M:%S");
  }
  else
  {
    Serial.println("Failed to obtain time");
  }
}

void setupCloudIoT() {
  deviceObject = new CloudIoTCoreDevice(CONFIG_PROJECT_ID,
                                        CONFIG_LOCATION,
                                        CONFIG_REGISTRY_ID,
                                        CONFIG_DEVICE_ID,
                                        CONFIG_PRIVATE_KEY);

  networkClientObject = new WiFiClientSecure();
  networkClientObject->setCACert(CONFIG_ROOT_CERT);

  mqttClientObject = new MQTTClient(CONFIG_MQTT_BUFFERS_SIZE);
  mqttClientObject->setOptions(CONFIG_MQTT_KEEPALIVE_TIMEOUT_SECS,
                               CONFIG_MQTT_CLEAN_SESSION,
                               CONFIG_MQTT_TIMEOUT_SECS);

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
  Serial.println("Received message");
  Serial.println("Topic   : " + topic);
  Serial.println("Payload : " + payload);
}

void messageReceivedAdvanced(MQTTClient *client, char topic[], char bytes[], int length) {
}

String getJwt() {
  unsigned long currentTime = 0;
  String jwt = "";

  currentTime = time(nullptr);
  jwt = deviceObject->createJWT(currentTime, CONFIG_JWT_EXPIRATION_SECS);
  return jwt;
}
