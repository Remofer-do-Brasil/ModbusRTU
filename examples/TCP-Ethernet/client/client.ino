/*
  ModbusTCP for W5x00 Ethernet library
  Basic Client code example

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <Ethernet.h> // Ethernet library v2 is required
#include <ModbusEthernet.h>
#include <SPI.h>

const uint16_t REG = 0;              // Holding-register offset.
IPAddress remote(192, 168, 13, 103); // Modbus server address.
const int32_t showDelay = 1000;      // Show the result every 1000 milliseconds.

// Enter a MAC address and IP address for your controller below.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress localIpAddress(192, 168, 13, 105); // The IP address will be dependent on your local network:
ModbusEthernet modbus;                       // Declare ModbusTCP instance

void setup() {
  Serial.begin(115200); // Start serial communication.
#ifdef AVR_LEONARDO
  while (!Serial) {
  } // wait for serial port to connect. Needed for Leonardo only
#endif
  Ethernet.init(17); // SS pin
  // start the Ethernet connection
  Ethernet.begin(mac);

  delay(1000); // Give the Ethernet shield one second to initialize.
  Serial.println("DHCP IP = ");
  Serial.println(Ethernet.localIP());
  modbus.client(); // Act as Modbus TCP server
}

uint16_t res = 0;
uint32_t showLast = 0;

void loop() {
  if (modbus.isConnected(remote)) {     // Check whether the Modbus server connection is established.
    modbus.readHreg(remote, REG, &res); // Read the holding register from the Modbus server.
    // Serial.println(res);
    delay(1000);
  } else {
    modbus.connect(remote); // Try to connect if necessary.
  }
  delay(100);                       // Pulling interval
  modbus.task();                    // Common local Modbus task
  if (millis() - showLast > 1000) { // Display register value every 5 seconds (with default settings)
    showLast = millis();
    Serial.println("Hreg = 0, value = ");
    Serial.println(res);
  }
}
