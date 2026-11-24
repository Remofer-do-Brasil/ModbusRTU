/*
  Modbus-Arduino Example - Test Holding Register (Modbus IP ESP8266)
  Read Switch Status on pin GPIO0
  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2017 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusIP_PICO.h>
#include <WiFi.h>

// Modbus register offsets.
const int SWITCH_ISTS = 100;
// Input pin.
const int switchPin = 0; // GPIO0.

// ModbusIP object.
ModbusIP modbus;

void setup() {
  Serial.begin(115200);

  WiFi.begin("your_ssid", "your_password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Configure Modbus IP.
  modbus.server();
  // Configure the input pin.
  pinMode(switchPin, INPUT);
  // Add SWITCH_ISTS register - Use addIsts() for digital inputs
  modbus.addIsts(SWITCH_ISTS);
}

void loop() {
  // Process Modbus events once per loop iteration.
  modbus.task();

  // Associate switchPin with the SWITCH_ISTS register.
  modbus.Ists(SWITCH_ISTS, digitalRead(switchPin));
  delay(10);
}
