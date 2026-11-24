/*
  Modbus Library for Arduino Example - Modbus IP Client (ESP8266/ESP32)
  Read Holding Register from Modbus Server in blocking way

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusIP_PICO.h>
#include <WiFi.h>

const int REG = 528;                // Holding-register offset.
IPAddress remote(192, 168, 30, 13); // Modbus server address.

ModbusIP modbus; // ModbusIP object.

void setup() {
  Serial.begin(115200);

  WiFi.begin("SSID", "PASSWORD");

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

void loop() {
  if (modbus.isConnected(remote)) {                      // Check whether the Modbus server connection is established.
    uint16_t trans = modbus.readHreg(remote, REG, &res); // Initiate Read Hreg from Modbus Server
    while (modbus.isTransaction(trans)) {                // Check if transaction is active
      modbus.task();
      delay(10);
    }
    Serial.println(res); // At this point res is filled with response value
  } else {
    modbus.connect(remote); // Try to connect if necessary.
  }
  delay(100);
}
