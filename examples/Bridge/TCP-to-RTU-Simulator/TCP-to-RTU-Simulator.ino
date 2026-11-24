/*
  ModbusRTU ESP8266/ESP32
  Modbus TCP-to-RTU bridge with an on-device Modbus RTU simulator.
*/

#include <WiFi.h>

#include <ModbusRTU.h>
#include <ModbusTCP.h>
// #include <SoftwareSerial.h>
// SoftwareSerial softwareSerial(13, 15);
#include <StreamBuf.h>
#define BSIZE 1024
uint8_t buf1[BSIZE];
uint8_t buf2[BSIZE];
StreamBuf firstStreamBuffer(buf1, BSIZE);
StreamBuf secondStreamBuffer(buf2, BSIZE);
DuplexBuf firstDuplexBuffer(&firstStreamBuffer, &secondStreamBuffer);
DuplexBuf secondDuplexBuffer(&secondStreamBuffer, &firstStreamBuffer);
ModbusRTU sym;

int DE_RE = 2;

ModbusRTU rtu;
ModbusTCP tcp;

IPAddress sourceIpAddress;

uint16_t activeTransactionId = 0; // Currently executed ModbusTCP transaction
uint8_t activeServerId = 0;       // Current request slave

bool cbTcpTrans(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Modbus transaction callback.
  (void)transactionId;
  (void)data;
  if (event != Modbus::EX_SUCCESS)                                             // If the transaction failed,
    Serial.printf("Modbus result: %02X, Mem: %d\n", event, ESP.getFreeHeap()); // display the Modbus error code.
  if (event == Modbus::EX_TIMEOUT) {                                           // If the transaction timed out,
    tcp.disconnect(tcp.eventSource());                                         // close the connection.
  }
  return true;
}

bool cbRtuTrans(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  (void)transactionId;
  (void)data;
  if (event != Modbus::EX_SUCCESS)                                             // If the transaction failed,
    Serial.printf("Modbus result: %02X, Mem: %d\n", event, ESP.getFreeHeap()); // display the Modbus error code.
  return true;
}

// Receive raw data from Modbus TCP.
Modbus::ResultCode cbTcpRaw(uint8_t* data, uint8_t len, void* custom) {
  auto src = (Modbus::frame_arg_t*)custom;

  Serial.print("TCP IP: ");
  Serial.print(IPAddress(src->ipAddress));
  Serial.printf(" Fn: %02X, len: %d \n\r", data[0], len);

  if (activeTransactionId) {                   // New TCP requests cannot be processed while awaiting an RTU response.
    tcp.setTransactionId(activeTransactionId); // Restore the transaction identifier from the incoming request.
    tcp.errorResponse(src->ipAddress, (Modbus::FunctionCode)data[0], Modbus::EX_SLAVE_DEVICE_BUSY);
    return Modbus::EX_SLAVE_DEVICE_BUSY;
  }

  rtu.rawRequest(activeServerId, data, len, cbRtuTrans);

  if (src->unitId) {
    tcp.setTransactionId(activeTransactionId); // Restore the transaction identifier from the incoming request.

    // uint16_t succeed = tcp.rawResponse(src->ipAddress, data, len, activeServerId);

    tcp.errorResponse(src->ipAddress, (Modbus::FunctionCode)data[0], Modbus::EX_ACKNOWLEDGE);
    return Modbus::EX_ACKNOWLEDGE;
  }

  sourceIpAddress = src->ipAddress;

  activeServerId = src->slaveId;

  activeTransactionId = src->transactionId;

  return Modbus::EX_SUCCESS;
}

// Receive raw data from Modbus TCP and forward it to the active Modbus RTU client.
Modbus::ResultCode cbRtuRaw(uint8_t* data, uint8_t len, void* custom) {
  auto src = (Modbus::frame_arg_t*)custom;
  tcp.setTransactionId(activeTransactionId); // Restore the transaction identifier from the incoming request.
  uint16_t succeed = tcp.rawResponse(sourceIpAddress, data, len, activeServerId);
  if (!succeed) {
    Serial.print("fail");
  }
  Serial.printf("RTU server: %d, function: %02X, length: %d, ", src->slaveId, data[0], len);
  Serial.print("TCP response IP: ");
  Serial.println(sourceIpAddress);

  activeTransactionId = 0;
  activeServerId = 0;
  return Modbus::EX_PASSTHROUGH;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin("E2", "*****");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  tcp.server();        // Initialize the Modbus TCP server.
  tcp.onRaw(cbTcpRaw); // Assign the raw-data callback.

  // softwareSerial.begin(19200, SWSERIAL_8E1);
  // rtu.begin(&serialPort, DE_RE); // Specify the RE/DE control pin.
  sym.begin((Stream*)&secondDuplexBuffer);
  sym.slave(1);
  sym.addHreg(1, 100);
  rtu.begin((Stream*)&firstDuplexBuffer); // No RE/DE control pin is required for the simulated link.
  rtu.client();                           // Initialize the Modbus RTU client.
  rtu.onRaw(cbRtuRaw);                    // Assign the raw-data callback.
}

void loop() {
  sym.task();
  rtu.task();
  tcp.task();
  yield();
}
