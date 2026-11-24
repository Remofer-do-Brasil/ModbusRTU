/*
    Modbus Library for Arduino
        ModbusRTU
    Copyright (C) 2019-2022 Alexander Emelianov (a.m.emelianov@gmail.com)
        https://github.com/emelianov/modbus-esp8266
        This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/
#pragma once
#include "ModbusAPI.h"

class ModbusRTUTemplate : public Modbus {
protected:
  Stream* _port;
  int16_t _txPin = -1;
#ifdef MODBUSRTU_REDE
  int16_t _rxPin = -1;
#endif
  bool _direct = true;       // Transmit control logic (true=direct, false=inverse)
  uint32_t _interFrameDelay; // Inter-frame delay in microseconds.
#ifdef MODBUSRTU_FLUSH_DELAY
  uint32_t _characterSendTime; // Character transmission time.
#endif
  uint32_t lastByteTimestamp = 0;
  bool isMaster = false;
  uint8_t _slaveId;
  uint32_t _timestamp = 0;
  cbTransaction _transactionCallback = nullptr;
  uint8_t* _data = nullptr;
  uint8_t* _sentFrame = nullptr;
  TAddress _sentReg = COIL(0);
  uint8_t sourceAddress = 0;

  uint16_t send(uint8_t slaveId, TAddress startRegister, cbTransaction callbackFunction, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr,
                bool waitResponse = true);
  // Prepare and send a Modbus RTU frame. The _frame buffer and _len must contain the Modbus data.
  // slaveId - server identifier
  // startRegister - first local register in which to store returned data; ignored for writes to a server
  // callbackFunction - transaction callback function
  // data - if not null, store returned data in this buffer instead of local registers
  bool rawSend(uint8_t slaveId, uint8_t* frame, uint8_t len);
  bool cleanup(); // Remove timed-out transactions.
  uint16_t crc16(uint8_t address, uint8_t* frame, uint8_t pduLength);
  uint16_t crc16_alt(uint8_t address, uint8_t* frame, uint8_t pduLen);

public:
  void setBaudrate(uint32_t baud = -1);
  uint32_t calculateMinimumInterFrameTime(uint32_t baud, uint8_t characterBits = 11);
  void setInterFrameTime(uint32_t interFrameTimeUs);
  uint32_t charSendTime(uint32_t baud, uint8_t characterBits = 11);
  template <class PortType> bool begin(PortType* port, int16_t txPin = -1, bool direct = true);
#if defined(MODBUSRTU_REDE)
  template <class PortType> bool begin(PortType* port, int16_t txPin, int16_t rxPin, bool direct);
#endif
  bool begin(Stream* port, int16_t txPin = -1, bool direct = true);
  void task();
  void client() { isMaster = true; };
  inline void master() { client(); }
  void server(uint8_t serverId) { _slaveId = serverId; };
  inline void slave(uint8_t slaveId) { server(slaveId); }
  uint8_t server() { return _slaveId; }
  inline uint8_t slave() { return server(); }
  uint32_t eventSource() override { return sourceAddress; }
};

template <class PortType> bool ModbusRTUTemplate::begin(PortType* port, int16_t txPin, bool direct) {
  uint32_t baud = 0;
#if defined(ESP32) || defined(ESP8266) // baudRate() only available with ESP32+ESP8266
  baud = port->baudRate();
#else
  baud = 9600;
#endif
  setInterFrameTime(calculateMinimumInterFrameTime(baud));
#if defined(MODBUSRTU_FLUSH_DELAY)
  _characterSendTime = charSendTime(baud);
#endif
  _port = port;
  if (txPin >= 0) {
    _txPin = txPin;
    _direct = direct;
    pinMode(_txPin, OUTPUT);
    digitalWrite(_txPin, _direct ? LOW : HIGH);
  }
  return true;
}
#if defined(MODBUSRTU_REDE)
template <class PortType> bool ModbusRTUTemplate::begin(PortType* port, int16_t txPin, int16_t rxPin, bool direct) {
  begin(port, txPin, direct);
  if (rxPin > 0) {
    _rxPin = rxPin;
    pinMode(_rxPin, OUTPUT);
    digitalWrite(_rxPin, _direct ? LOW : HIGH);
  }
}
#endif
class ModbusRTU : public ModbusAPI<ModbusRTUTemplate> {};
