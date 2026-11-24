/*
  Modbus-Arduino Example - Publish multiple digital inputs as coils (Modbus IP ESP8266/ESP32)

  Original library
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino

  Current version
  (c)2018 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  Current version
  (C) 2026 Jimes Yang (admin@sndnvaps.com)
  https://github.com/sndnvaps/modbus-pico
*/

#include <WiFi.h>

#include <ModbusIP_PICO.h>

uint8_t pinList[] = { 12, 13, 14, 15, 16, 17, 18, 19, 21, 22 };

#define LEN       sizeof(pinList) / sizeof(uint8_t)
#define COIL_BASE 0
// ModbusIP object.
ModbusIP modbus;

// Callback that reads the corresponding digital input.
uint16_t cbRead(TRegister* reg, uint16_t value) {
  (void)value;
  // Check the address of the register that triggered the callback.
  // See Modbus.h for the TRegister and TAddress definitions.
  if (reg->address.address < COIL_BASE) return 0;
  uint8_t offset = reg->address.address - COIL_BASE;
  if (offset >= LEN) return 0;
  return COIL_VAL(digitalRead(pinList[offset]));
}
// Preserve the current value to make the mapped digital inputs read-only.
uint16_t cbWrite(TRegister* reg, uint16_t value) {
  (void)value;
  return reg->value;
}

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress localIpAddress) {
  Serial.println(localIpAddress);
  return true;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("ssid", "password");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  for (uint8_t pinIndex = 0; pinIndex < LEN; pinIndex++) pinMode(pinList[pinIndex], INPUT);
  modbus.onConnect(cbConn); // Assign the connection callback.
  modbus.server();

  modbus.addCoil(COIL_BASE, COIL_VAL(false), LEN); // Add the coils.
  modbus.onGetCoil(COIL_BASE, cbRead, LEN);        // Assign one read callback to all coils in the range.
  modbus.onSetCoil(COIL_BASE, cbWrite, LEN);       // Assign one write callback to all coils in the range.
}

void loop() {
  // Process Modbus events once per loop iteration.
  modbus.task();
  delay(10);
}
