/*
  Modbus-Arduino Example - Master Modbus IP Client (ESP8266/ESP32)
  Read Holding Register from Server device

  (c)2018 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <WiFi.h>

#include <ModbusIP_PICO.h>

#define WIFI_SSID     "TP-LINK_71A109"
#define WIFI_PASSWORD "447826004aZ"

const int REG = 0;                   // Holding-register offset.
IPAddress remote(192, 168, 13, 103); // Modbus server address.
const int LOOP_COUNT = 10;

ModbusIP modbus; // ModbusIP object.

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  modbus.client();
}

uint16_t res = 0;
uint32_t showLast = 0;

void loop() {
  if (modbus.isConnected(remote)) {     // Check whether the Modbus server connection is established.
    modbus.readHreg(remote, REG, &res); // Read the holding register from the Modbus server.
    // Serial.println(res);
    delay(1000);
  } else {
    modbus.connect(remote); // Try to connect if necessary.
  }
  delay(100);                       // Pulling interval
  modbus.task();                    // Common local Modbus task
  if (millis() - showLast > 1000) { // Display register value every 5 seconds (with default settings)
    showLast = millis();
    Serial.println("Hreg = 0, value = ");
    Serial.println(res);
  }
}
