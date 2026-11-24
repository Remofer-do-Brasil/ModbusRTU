/*
  ModbusTCP for W5x00 Ethernet library
  Basic Server code example

  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <Ethernet.h> // Ethernet library v2 is required
#include <ModbusEthernet.h>
#include <SPI.h>

// Modbus Registers Offsets
#define REG_TEMPERATURE 0 // 温度寄存器（放大10倍，25.5℃=255）
#define REG_HUMIDITY    1 // 湿度寄存器（放大10倍，60.2%=602）

// 模拟温湿度数据（存储在寄存器中）
uint16_t temperature = 256; // 25.6℃（放大10倍存储）
uint16_t humidity = 605;    // 60.5%（放大10倍存储）
// Enter a MAC address and IP address for your controller below.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress localIpAddress(192, 168, 13, 177); // The IP address will be dependent on your local network:
ModbusEthernet modbus;                       // Declare ModbusTCP instance

void setup() {

  // SPI.setRX(16);
  // SPI.setCS(17);
  // SPI.setSCK(18);
  // SPI.setTX(19);
  // interruptPin = 21

  Serial.begin(115200);                // Start serial communication.
  Ethernet.init(17);                   // SS pin
  Ethernet.begin(mac, localIpAddress); // start the Ethernet connection
  delay(1000);                         // Give the Ethernet shield one second to initialize.
  modbus.setMode(MODE_UDP);
  modbus.server(); // Act as Modbus TCP server
  modbus.addHreg(REG_TEMPERATURE);
  modbus.addHreg(REG_HUMIDITY);

  Serial.println("IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Modbus TCP-Ethernet服务器已启动");
}

void loop() {
  // 模拟数据更新（每1秒更新一次温湿度）
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate >= 2000) {
    lastUpdate = millis();
    // 温度随机波动±0.5℃
    temperature = constrain(temperature + (random(-5, 6)), 200, 300);
    // 湿度随机波动±0.5%
    humidity = constrain(humidity + (random(-5, 6)), 500, 700);
    // 更新寄存器

    modbus.Hreg(REG_TEMPERATURE, temperature);
    modbus.Hreg(REG_HUMIDITY, humidity);

    // 调试输出
    Serial.printf("Temperature: %.1f℃, Humidity: %.1f%%\n", temperature / 10.0, humidity / 10.0);
  }
  modbus.task(); // Server Modbus TCP queries
  delay(10);
}
