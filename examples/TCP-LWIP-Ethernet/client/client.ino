/*
  ModbusTCP for W5x00 Ethernet library
  Basic Client code example

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <SPI.h>
// #include <Ethernet.h> // Ethernet library v2 is required.

#include <EthernetCompat.h> // lwIP-Ethernet compatibility layer.
#include <ModbusLWIP_Ethernet.h>

// chipSelectPin = 17
// interruptPin = 21
ArduinoWiznet5500lwIP Ethernet(17, SPI, 21);
// ArduinoWiznet5100lwIP Ethernet(chipSelectPin, SPI, interruptPin);
// ArduinoENC28J60lwIP Ethernet(chipSelectPin, SPI, interruptPin);

const uint16_t REG = 0;              // Holding-register offset.
IPAddress remote(192, 168, 13, 103); // Modbus server address.
const int32_t showDelay = 2000;      // Show the result every 2000 milliseconds.

// Enter a MAC address and IP address for your controller below.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
IPAddress localIpAddress(192, 168, 13, 178); // The IP address will be dependent on your local network:
IPAddress myDns(192, 168, 13, 1);            // DNS server address.

ModbusEthernet modbus; // Declare ModbusTCP instance

void setup() {

  // SPI.setRX(16);
  // SPI.setCS(17);
  // SPI.setSCK(18);
  // SPI.setTX(19);
  // interruptPin = 21

  Serial.begin(115200); // Start serial communication.

  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // Wait because the application cannot continue without Ethernet hardware.
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // Configure a static IP address if DHCP fails.
    Ethernet.begin(mac, localIpAddress, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  delay(1000);     // Give the Ethernet shield one second to initialize.
  modbus.client(); // Act as Modbus TCP server
}

uint16_t res = 0;
uint32_t showLast = 0;

void loop() {
  if (modbus.isConnected(remote)) {     // Check whether the Modbus server connection is established.
    modbus.readHreg(remote, REG, &res); // Read the holding register from the Modbus server.
    Serial.println(res);
    delay(1000);
  } else {
    modbus.connect(remote); // Try to connect if necessary.
  }
  delay(100);                       // Pulling interval
  modbus.task();                    // Common local Modbus task
  if (millis() - showLast > 1000) { // Display register value every 5 seconds (with default settings)
    showLast = millis();
    Serial.println("HReg = ");
    Serial.println(res);
  }
}
