#include "heltec.h"
#include <WiFi.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const char *ssid     = "UTconsole";
const char *password = "UtahTechConsole";

const char *host = "home.holt.dj";

DHT_Unified dht(21, DHT11);

void printToScreen(String s) {
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, s);
  Heltec.display->display();
}

void setup() {
  Heltec.begin(true /*display*/, false /*LoRa*/, true /*Serial*/);
  printToScreen("READY");
  delay(1000);

  printToScreen("MAC address:\n" + WiFi.macAddress());
  delay(1000);

  printToScreen("Connecting to WiFi:\n" + String(ssid));
  delay(1000);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  printToScreen("WiFi connected!\nIP: " + WiFi.localIP().toString());
  delay(1000);

  dht.begin();
}

void loop() {
  // postSensorData();
  // delay(30000);
}

void postSensorData() {
  sensors_event_t temperature_event;
  sensors_event_t humidity_event;
  dht.humidity().getEvent(&temperature_event);
  dht.temperature().getEvent(&humidity_event);

  if (!isnan(temperature_event.temperature) && !isnan(humidity_event.relative_humidity)) {
    String location = "DJESP32";
    String temperature = String(temperature_event.temperature);
    String humidity = String(humidity_event.relative_humidity);

    String dataToSend = "location=" + location + "&temperature=" + temperature + "&humidity=" + humidity;

    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      printToScreen("Connection failed!");
      return;
    }

    String url = "/weatherapi/readings";

    printToScreen("Connected!\nRequesting path:\n" + url);
    delay(1000);

    String length = String(dataToSend.length());
    String req = String("POST ") + url + " HTTP/1.1\r\n" +
      "Host: " + host + "\r\n" +
      "Connection: close\r\n" +
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Length: " + length + "\r\n\r\n" +
      dataToSend;
    client.print(req);

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        printToScreen("Client timeout!");
        client.stop();
      }
    }
  }
}