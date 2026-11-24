/*
  Modbus TCP server for the ClearCore Arduino wrapper.

  (c)2021 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <Ethernet.h> // Ethernet library v2 is required.

#include <ModbusAPI.h>
#include <ModbusTCPTemplate.h>

class ModbusEthernet : public ModbusAPI<ModbusTCPTemplate<EthernetServer, EthernetClient>> {};

const uint16_t REG = 512;       // Holding-register offset.
const int32_t showDelay = 5000; // Show the result every 5000 milliseconds.

bool usingDhcp = true;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE }; // Controller MAC address.
IPAddress localIpAddress(192, 168, 30, 178);         // Choose an address suitable for the local network.
ModbusEthernet modbus;                               // Modbus TCP instance.

// Callback function for client connect. Returns true to allow connection.
bool cbConn(IPAddress localIpAddress) {
  Serial.println(localIpAddress);
  return true;
}

void setup() {
  Serial.begin(9600);
  uint32_t timeout = 5000;
  uint32_t startTime = millis();
  while (!Serial && millis() - startTime < timeout) continue;

  // Get the Ethernet module up and running.
  if (usingDhcp) {
    int dhcpSuccess = Ethernet.begin(mac);
    if (dhcpSuccess) Serial.println("DHCP configuration was successful.");
    else {
      Serial.println("DHCP configuration was unsuccessful!");
      Serial.println("Try again using a manual configuration...");
      while (true) continue;
    }
  } else {
    Ethernet.begin(mac, localIpAddress);
  }

  // Make sure the physical link is up before continuing.
  while (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("The Ethernet cable is unplugged...");
    delay(1000);
  }
  modbus.server(); // Act as a Modbus TCP server.
  modbus.onConnect(cbConn);
  modbus.addHreg(REG); // Expose the holding register.
}

void loop() {
  modbus.task(); // Process Modbus events.
  delay(10);
}
