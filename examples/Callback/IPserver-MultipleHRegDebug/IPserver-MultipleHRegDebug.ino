/*
  Modbus-Arduino Example - Debug multiple holding registers (Modbus IP ESP8266/ESP32)

  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2018 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <WiFi.h>

#include <ModbusIP_PICO.h>

#define LEN 10

// ModbusIP object.
ModbusIP modbus;

// Callback that logs a register read.
uint16_t cbRead(TRegister* reg, uint16_t value) {
  Serial.print("Read. Reg RAW#: ");
  Serial.print(reg->address.address);
  Serial.print(" Old: ");
  Serial.print(reg->value);
  Serial.print(" New: ");
  Serial.println(value);
  return value;
}
// Callback that logs a register write.
uint16_t cbWrite(TRegister* reg, uint16_t value) {
  Serial.print("Write. Reg RAW#: ");
  Serial.print(reg->address.address);
  Serial.print(" Old: ");
  Serial.print(reg->value);
  Serial.print(" New: ");
  Serial.println(value);
  return value;
}

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress localIpAddress) {
  Serial.println(localIpAddress);
  return true;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("ssid", "pass");

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

  if (!modbus.addHreg(0, 0xF0F0, LEN)) Serial.println("Error"); // Add holding registers.
  modbus.onGetHreg(0, cbRead, LEN);                             // Assign the read callback to all holding registers.
  modbus.onSetHreg(0, cbWrite, LEN);
}

void loop() {
  // Process Modbus events once per loop iteration.
  modbus.task();
  delay(10);
}
