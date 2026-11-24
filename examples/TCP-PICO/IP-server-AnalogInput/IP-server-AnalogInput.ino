/*
  Modbus-Arduino Example - Test Analog Input (Modbus IP ESP8266)
  Read Analog sensor on Pin ADC (ADC input between 0 ... 1V)
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
const int SENSOR_IREG = 100;

// ModbusIP object.
ModbusIP modbus;

long lastUpdate;

void setup() {
  Serial.begin(115200);

  WiFi.begin("your_ssid", "your_password");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  modbus.server(); // Start the Modbus IP server.
  // Add SENSOR_IREG register - Use addIreg() for analog Inputs
  modbus.addIreg(SENSOR_IREG);

  lastUpdate = millis();
}

void loop() {
  // Process Modbus events once per loop iteration.
  modbus.task();

  // Read every two seconds.
  if (millis() > lastUpdate + 2000) {
    lastUpdate = millis();
    // Store the raw value (0-1024).
    modbus.Ireg(SENSOR_IREG, analogRead(A0));
  }
  delay(10);
}
