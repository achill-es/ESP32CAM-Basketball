/*
 *    publisher to topic on cloud broker
 */

#include <ESP32QRCodeReader.h>
#include <string.h>

#include "credentials.h"

#define LED_PIN (4)

// QR code reader
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

// parses the matchId from a decoded URL
String parse_matchId(const char *url) {
  String reads = url;

  int16_t index1 = reads.indexOf("matchId=");                // finds location of "matchId=" parameter
  int16_t index2 = reads.indexOf('&', index1 + search_len);  // finds location of ampersand, if any further parameter has been appended

  if (index2 - index1 == (int16_t)(search_len + mqtt_mlen))  // matchId followed by further parameter
    return reads.substring(index1 + (int16_t)search_len, index2);
  else if (index2 == -1)  // matchId not followed by further parameter
    return reads.substring(index1 + (int16_t)search_len);
  else {
    Serial.println("no legal matchId found!");
    return "\0";
  }
}

// this callback function is called whenever a new QR code has been read
void onQrCodeTask(void *pvParameters) {
  struct QRCodeData qrCodeData;
  uint8_t buf[mqtt_mlen+1];

  while (true) {
    if (reader.receiveQrCode(&qrCodeData, 100)) {
      Serial.println("Found QRCode");

      if (qrCodeData.valid) {
        Serial.printf("Valid payload: %s\n", (const char *)qrCodeData.payload);

        String message = parse_matchId((const char *)qrCodeData.payload);

        if (message.length() == mqtt_mlen) {
          Serial.printf("Message to topic: %s\n", message.c_str());
          message.getBytes(buf, mqtt_mlen+1);
          mqttClient.publish(mqtt_topic, buf, mqtt_mlen, true);  // publish to the topic retained

          digitalWrite(LED_PIN, HIGH);  // LED on
          delay(100);                   //      "wait 100msec" means "flash LED for 0.1sec"
          digitalWrite(LED_PIN, LOW);   // LED off
        } else {
          Serial.printf("length of matchId = %d\n", message.length());
        }
      } else {
        Serial.printf("Invalid payload: %s\n", (const char *)qrCodeData.payload);
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// the setup function does the main things in initializing everything
void setup() {
  // set software serial baud to 115200
  Serial.begin(115200);
  // declare LED's PIN for output
  pinMode(LED_PIN, OUTPUT);
  // connect to WiFi and MQTT for the first time
  reconnect(true, true);
  // initialize our QR code reader
  reader.setup();
  Serial.println("Setup QRCode Reader");
  reader.beginOnCore(1);
  Serial.println("Begin on Core 1");
  // register a task to execute whenever QR code is read
  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

// the main loop is quite "empty": it just checks whether reconnect required
void loop() {
  delay(1000);
  if (!mqttClient.connected()) {
    reconnect(false, true);                                          // make sure we get reconnected when connection to MQTT broker is lost
    xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);  // re-register our task
  }
  mqttClient.loop();
}

// end of code file