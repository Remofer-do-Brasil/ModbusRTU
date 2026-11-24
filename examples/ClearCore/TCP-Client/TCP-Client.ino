/*
  Modbus TCP client for the ClearCore Arduino wrapper.

  (c)2021 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <Ethernet.h> // Ethernet library v2 is required.

#include <ModbusAPI.h>
#include <ModbusTCPTemplate.h>

class ModbusEthernet : public ModbusAPI<ModbusTCPTemplate<EthernetServer, EthernetClient>> {};

const uint16_t REG = 512;           // Holding-register offset.
IPAddress remote(192, 168, 30, 12); // Modbus server address.
const int32_t showDelay = 5000;     // Show the result every 5000 milliseconds.

bool usingDhcp = true;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE }; // Controller MAC address.
IPAddress localIpAddress(192, 168, 30, 178);         // Choose an address suitable for the local network.
ModbusEthernet modbus;                               // Modbus TCP instance.

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
  modbus.client(); // Act as a Modbus TCP client.
}

uint16_t res = 0;
uint32_t showLast = 0;

void loop() {
  if (modbus.isConnected(remote)) {     // Check whether the Modbus server connection is established.
    modbus.readHreg(remote, REG, &res); // Read the holding register from the Modbus server.
  } else {
    modbus.connect(remote); // Try to connect if necessary.
  }
  delay(100);                            // Polling interval.
  modbus.task();                         // Process Modbus events.
  if (millis() - showLast > showDelay) { // Display register value every 5 seconds (with default settings)
    showLast = millis();
    Serial.println(res);
  }
}
