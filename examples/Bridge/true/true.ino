/*
  Modbus RTU ESP8266/ESP32
  Full RTU-to-TCP bridge example.

  (c)2021 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <ModbusRTU.h>
#include <ModbusTCP.h>
#include <WiFi.h>
#include <vector>

ModbusRTU rtu;
ModbusTCP tcp;

// Modbus RTU server identifier to Modbus TCP IP-address mapping.
struct ServerMapping {
  uint8_t serverId;
  IPAddress ipAddress;
  uint8_t unitId = MODBUSIP_UNIT;
  ServerMapping(uint8_t serverIdentifier, IPAddress remoteAddress, uint8_t unitIdentifier = MODBUSIP_UNIT) {
    serverId = serverIdentifier;
    ipAddress = remoteAddress;
    unitId = unitIdentifier;
  };
};
std::vector<ServerMapping> mappings;
uint16_t activeTransactionId = 0;
uint8_t activeServerId = 0;

bool cbTcpTrans(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Modbus transaction callback.
  (void)transactionId;
  (void)data;
  if (event != Modbus::EX_SUCCESS) Serial.printf("Modbus result: %02X, Mem: %d\n", event, ESP.getFreeHeap());
  if (event == Modbus::EX_TIMEOUT) {
    tcp.disconnect(tcp.eventSource());
  }
  return true;
}

// Receive raw data from Modbus TCP and forward it to the active Modbus RTU client.
Modbus::ResultCode cbTcpRaw(uint8_t* data, uint8_t len, void* custom) {
  auto src = (Modbus::frame_arg_t*)custom;
  Serial.print("TCP IP: ");
  Serial.print(IPAddress(src->ipAddress));
  Serial.printf(" Fn: %02X, len: %d \n", data[0], len);
  if (!src->toServer && activeTransactionId == src->transactionId) {
    rtu.rawResponse(activeServerId, data, len);
  } else return Modbus::EX_PASSTHROUGH; // Allow the generic Modbus TCP routines to process the frame.
  activeTransactionId = 0;
  activeServerId = 0;
  return Modbus::EX_SUCCESS; // Stop further processing.
}

// Receive raw data from Modbus RTU.
Modbus::ResultCode cbRtuRaw(uint8_t* data, uint8_t len, void* custom) {
  auto src = (Modbus::frame_arg_t*)custom;
  Serial.printf("RTU server: %d, function: %02X, length: %d, ", src->slaveId, data[0], len);
  auto mappingEntry = std::find_if(mappings.begin(), mappings.end(), [src](ServerMapping& item) { return item.serverId == src->slaveId; });
  if (mappingEntry != mappings.end()) {
    if (!tcp.isConnected(mappingEntry->ipAddress)) {
      if (!tcp.connect(mappingEntry->ipAddress)) {
        Serial.printf("Error: connection timeout\n");

        rtu.errorResponse(mappingEntry->serverId, (Modbus::FunctionCode)data[0], Modbus::EX_DEVICE_FAILED_TO_RESPOND);
        // Note:
        // With the default library settings on both sides, an RTU client that initiates a request through the bridge may receive EX_TIMEOUT instead
        // of EX_DEVICE_FAILED_TO_RESPOND because both the connection and RTU response timeouts are one second.
        return Modbus::EX_DEVICE_FAILED_TO_RESPOND; // Stop processing the frame.
      }
    }
    // Save the transaction and server identifiers for response processing.
    activeTransactionId = tcp.rawRequest(mappingEntry->ipAddress, data, len, cbTcpTrans, mappingEntry->unitId);
    if (!activeTransactionId) {
      tcp.disconnect(mappingEntry->ipAddress);
      Serial.printf("Send failed\n");
      rtu.errorResponse(mappingEntry->serverId, (Modbus::FunctionCode)data[0], Modbus::EX_DEVICE_FAILED_TO_RESPOND);
      return Modbus::EX_DEVICE_FAILED_TO_RESPOND; // Stop processing the frame.
    }
    Serial.printf("Transaction: %d\n", activeTransactionId);
    activeServerId = mappingEntry->serverId;
    return Modbus::EX_SUCCESS; // Stop processing the frame.
  }
  Serial.printf("Ignored: no mapping\n");
  return Modbus::EX_PASSTHROUGH; // Use the generic Modbus RTU routines when no mapping is found.
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  tcp.client();        // Initialize the Modbus TCP client.
  tcp.onRaw(cbTcpRaw); // Assign the raw-data callback.

  Serial1.begin(9600, SERIAL_8N1, 18, 19);
  rtu.begin(&Serial1);
  rtu.server(3);       // Initialize the Modbus RTU server.
  rtu.onRaw(cbRtuRaw); // Assign the raw-data callback.

  // Assign server mappings.
  mappings.push_back({ 1, IPAddress(192, 168, 30, 18) });
  mappings.push_back({ 2, IPAddress(192, 168, 30, 19) });
}

void loop() {
  rtu.task();
  tcp.task();
  yield();
}
