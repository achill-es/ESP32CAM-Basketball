/*
 *    subscriber to topic on cloud broker
 */

#include <Inkplate.h>

#include "credentials.h"

#define URL_LEN (255)
#define MQTT_QOS1 (1)

// template for the Google API to convert URL into QR Code (format PNG, size 547x547 pixels):
static const char googleapi[] PROGMEM = "http://chart.apis.google.com/chart?chs=547x547&cht=qr&chld=L|0&chl=https%%3A%%2F%%2Fscoresheets.nbn23.com%%2Fscoresheet-5v5%%3FmatchId%%3D%16.16s%%26language%%3Den";

// the generated API call string
char apicall[URL_LEN];

// Inkplate6 e-ink display in B/W mode
Inkplate display(INKPLATE_1BIT);

// this callback function is called whenever a new message has been published to the topic
void callback(const char topic[], byte* payload, unsigned int length) {
  // don't process if wrong message length
  if (length != mqtt_mlen)
    Serial.println("Probably illegal matchId arrived in topic!");
  else {  
    // show on console what message = matchId has arrived
    Serial.printf("Message arrived in topic: %s\n", topic);
    Serial.print("Message: ");
    for (int i = 0; i < length; i++)
      Serial.print((char)payload[i]);
    Serial.println();

    // display matchId in bottom right corner of e-ink display
    payload[mqtt_mlen] = '\0';
    display.setTextSize(2);
    display.setCursor(7, 579);
    display.print((char *)payload);

    // generate the API call string from the template and the topic message
    snprintf(apicall, URL_LEN, googleapi, (char*)payload);
    Serial.printf("Call Google API with: %s\n", apicall);

    // download the QR code using the API call into the buffer
    Serial.print("Return code drawImage: ");
    Serial.println(display.drawImage(apicall, display.PNG, 112, 23));

    // and display it, that's all!
    display.display();
  }
}

// the setup function does the main things in initializing everything
void setup() {
  // set software serial baud to 115200
  Serial.begin(115200);
  // initialize our e-ink display
  display.begin();
  // connect to WiFi and MQTT for the first time
  reconnect(true, false);
  // register callback
  mqttClient.setCallback(callback);
  // subscribe to our topic
  mqttClient.subscribe(mqtt_topic, MQTT_QOS1);
}

// the main loop is quite "empty": it just checks whether reconnect required
void loop() {
  delay(1000);
  if (!mqttClient.connected())
  {
    // make sure we get reconnected when connection to MQTT broker is lost
    reconnect(false, false);  
    // register callback
    mqttClient.setCallback(callback);
    // subscribe to our topic
    mqttClient.subscribe(mqtt_topic, MQTT_QOS1);
  }
  mqttClient.loop();
}

// end of code file