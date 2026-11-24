/*
  Modbus-Arduino Example -  Modbus IP Client (ESP8266/ESP32)
  Control Led on D4/TX pin by remote Modbus device using Read Single Coil Modbus Function

  (c)2018 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <ModbusIP_PICO.h>
#include <WiFi.h>

const int LED_COIL = 1;              // Coil offset.
IPAddress remote(192, 168, 30, 116); // Modbus server address.

ModbusIP modbus; // ModbusIP object.

uint16_t onCoilChange(TRegister* reg, uint16_t value) { // Callback function.
  if (reg->value != value) {                            // Check whether the coil state will change.
    Serial.print("Set reg: ");
    Serial.println(value);
    if (COIL_BOOL(value)) {
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
  return value;
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("SSID", "password");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  modbus.client(); // Initialize local Modbus Client
  pinMode(LED_BUILTIN, OUTPUT);
  modbus.addCoil(LED_COIL);                 // Add Coil
  modbus.onSetCoil(LED_COIL, onCoilChange); // Assign the coil callback.
}

void loop() {
  if (modbus.isConnected(remote)) {              // Check whether the Modbus server connection is established.
    modbus.pullCoil(remote, LED_COIL, LED_COIL); // Read the coil from the Modbus server.
  } else {
    modbus.connect(remote); // Try to connect if necessary.
  }
  modbus.task(); // Common local Modbus task
  delay(10);     // Polling interval
}
