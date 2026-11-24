/*
  Modbus-Arduino Example - Test an LED using a callback (Modbus IP ESP8266/ESP32)
  Control an LED using the Modbus Write Single Coil function.
  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2017 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <WiFi.h>

#include <ModbusIP_PICO.h>

// Modbus register offsets.
const int LED_COIL = 100;

const int ledPin = LED_BUILTIN; // Pico LED_BUILTIN.

// ModbusIP object.
ModbusIP modbus;

// Callback for writing the coil. Returns the value to store.
uint16_t cbLed(TRegister* reg, uint16_t value) {
  (void)reg;
  // Associate ledPin with the LED_COIL register.
  digitalWrite(ledPin, COIL_BOOL(value));
  return value;
}

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress localIpAddress) {
  Serial.println(localIpAddress);
  return true;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("SID", "PASSWORD");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  modbus.onConnect(cbConn); // Assign the connection callback.
  modbus.server();

  pinMode(ledPin, OUTPUT);
  modbus.addCoil(LED_COIL);          // Add the coil.
  modbus.onSetCoil(LED_COIL, cbLed); // Assign the callback that handles writes to LED_COIL.
}

void loop() {
  // Process Modbus events once per loop iteration.
  modbus.task();
  delay(10);
}
