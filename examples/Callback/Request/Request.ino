/*
  Modbus-Arduino Example - Use request-processing callbacks to control holding-register writes.

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <WiFi.h>

#include <ModbusTCP.h>

const uint16_t RO_FLAG = 100; // Coil that enables or disables holding-register writes.
const uint16_t RW_HREG = 200; // Sample holding register.

// Modbus TCP object.
ModbusTCP modbus;

Modbus::ResultCode cbPreRequest(Modbus::FunctionCode functionCode, const Modbus::RequestData data) {
  (void)data;
  Serial.printf("PRE Function: %02X\n", functionCode);
  if ((functionCode == Modbus::FC_WRITE_REG || functionCode == Modbus::FC_WRITE_REGS) && modbus.Coil(RO_FLAG)) return Modbus::EX_ILLEGAL_FUNCTION;
  return Modbus::EX_SUCCESS;
}

Modbus::ResultCode cbPostRequest(Modbus::FunctionCode functionCode, const Modbus::RequestData data) {
  (void)data;
  Serial.printf("POST Function: %02X\n", functionCode);
  return Modbus::EX_SUCCESS;
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
  modbus.onRequest(cbPreRequest);
  modbus.onRequestSuccess(cbPostRequest);
  modbus.server();

  modbus.addCoil(RO_FLAG);
  modbus.addHreg(RW_HREG, 100);
}

void loop() {
  modbus.task();
  delay(10);
}
