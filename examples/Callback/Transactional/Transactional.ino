/*
  Modbus-Arduino Example - Modbus IP Client (ESP8266/ESP32)
  Write multiple coils to a Modbus server.

  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#include <WiFi.h>

#include <ModbusIP_PICO.h>

const int REG = 100;                 // First Modbus coil offset.
const int COUNT = 5;                 // Number of coils.
IPAddress remote(192, 168, 20, 102); // Modbus server address.

ModbusIP modbus; // ModbusIP object.

void setup() {
  Serial.begin(115200);

  WiFi.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  modbus.client();
}

bool transactionCallback(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Modbus transaction callback.
  (void)transactionId;
  (void)data;
  if (event != Modbus::EX_SUCCESS)                 // If the transaction failed,
    Serial.printf("Modbus result: %02X\n", event); // display the Modbus error code.
  if (event == Modbus::EX_TIMEOUT) {               // If the transaction timed out,
    modbus.disconnect(remote);                     // close the connection
    modbus.dropTransactions();                     // and cancel all pending transactions.
  }
  return true;
}

bool coilValues[COUNT] = { false, true, false, true, true };

void loop() {
  if (!modbus.isConnected(remote)) { // Check whether the Modbus server connection is established.
    modbus.connect(remote);          // Try to connect if necessary.
    Serial.print(".");
  }
  if (!modbus.writeCoil(remote, REG, coilValues, COUNT, transactionCallback)) // Try to write COUNT coils to the Modbus server.
    Serial.print("#");
  modbus.task(); // Process Modbus events.
  delay(50);     // Write interval.
}
