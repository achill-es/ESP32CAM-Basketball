/*
 *  module to (re)connect to both WiFi and then cloud MQTT broker
 */

#include <WiFiClientSecure.h>
#include <string>

#include "credentials.h"

#define WAITxSEC (5000)

// init a secured WiFi client
static WiFiClientSecure wlanClient;

// use WiFi client to init a MQTT client
PubSubClient mqttClient(wlanClient);

void reconnect(bool fromsetup, bool ispub) {
  // connecting to a WiFi network
  if (WiFi.status() != WL_CONNECTED) {
    do {
      WiFi.begin(wifi_ssid, wifi_password);
      Serial.printf("%sonnecting to WiFi...\n",(fromsetup ? "C" : "Rec"));
      delay(WAITxSEC);
    } while (WiFi.status() != WL_CONNECTED);
    Serial.printf("%sonnected to the WiFi network.\n", (fromsetup ? "C" : "Rec"));
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  }

  if (fromsetup) {
    // set root CA certificate for implicit checking
    wlanClient.setCACert(mqtt_cert);
    // set our MQTT cloud broker
    mqttClient.setServer(mqtt_broker, mqtt_port);
  }

  // connecting to MQTT broker
  while (!mqttClient.connected()) {
    char mqtt_clientid[mqtt_clen];

    sprintf(mqtt_clientid, "%c%s-%s", (ispub ? 'p' : 's'), infix_mqttcID, WiFi.macAddress().c_str());
    Serial.printf("Client %s %sconnects to the cloud MQTT broker...\n", mqtt_clientid, fromsetup ? "" : "re");

    if (mqttClient.connect(mqtt_clientid, mqtt_username, mqtt_password)) {
      Serial.printf("%sonnected to cloud MQTT broker.\n", (fromsetup ? "C" : "Rec"));
    } else {
      Serial.printf("Failed to %sconnect to cloud MQTT broker, rc = %d.\n", fromsetup ? "" : "re", mqttClient.state());
      Serial.println("Retrying in a few seconds...");
      delay(WAITxSEC);
    }
  }
}

// end of code file