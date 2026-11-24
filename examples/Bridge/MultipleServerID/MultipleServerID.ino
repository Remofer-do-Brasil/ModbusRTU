/*
  ModbusRTU ESP32
  Minimal example of one server responding to multiple identifiers so that it appears as multiple devices on the bus.

  (c)2022 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <ModbusRTU.h>

#define HOLDING_REGISTER 10
#define PRIMARY_ID       1
#define PRIMARY_VALUE    100
#define SECONDARY_ID     2
#define SECONDARY_VALUE  200

ModbusRTU modbus;

Modbus::ResultCode cbRtuRaw(uint8_t* data, uint8_t len, void* custom) {
  auto src = (Modbus::frame_arg_t*)custom; // The custom argument contains metadata for the incoming packet.
  Serial.printf("RTU server: %d, function: %02X, length: %d, ", src->slaveId, data[0], len);
  if (src->slaveId == SECONDARY_ID)  // Check whether the packet is addressed to SECONDARY_ID.
    return Modbus::EX_FORCE_PROCESS; // Force processing because the packet is not addressed to PRIMARY_ID.

  return Modbus::EX_PASSTHROUGH; // or process packet normally
}

uint16_t cbRead(TRegister* reg, uint16_t value) {
  if (modbus.eventSource() == SECONDARY_ID) return SECONDARY_VALUE;
  return value;
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  modbus.begin(&Serial1);
  modbus.slave(PRIMARY_ID); // Set Modbus to work as a server with ID <PRIMARY_ID>
  modbus.onRaw(cbRtuRaw);   // Assign raw packet callback
  modbus.addHreg(HOLDING_REGISTER, PRIMARY_VALUE);
  modbus.onGetHreg(HOLDING_REGISTER, cbRead);
}

void loop() {
  modbus.task();
  yield();
}
