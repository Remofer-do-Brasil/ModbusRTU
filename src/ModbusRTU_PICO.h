// Copyright (C) 2019-2022 Alexander Emelianov. See LICENSE.txt.

#ifndef MODBUS_RTU_PICO_H
#define MODBUS_RTU_PICO_H

#include "Modbus.h"
#include <hardware/uart.h>

struct ModbusUartConfig {
  uart_inst_t* uart{};
  unsigned txPin{};
  unsigned rxPin{};
  uint32_t baud{};
  uart_parity_t parity = UART_PARITY_EVEN; // NONE selects two stop bits
  int directionPin = -1;                   // Optional RS-485 DE or tied DE and /RE
  bool transmitLevel = true;
  int receiverEnablePin = -1; // Optional separate /RE (active low)
  bool receiveLevel = false;
};

class ModbusRTU : public Modbus {
public:
  ModbusRTU() = default;
  ModbusRTU(const ModbusRTU&) = delete;
  ModbusRTU& operator=(const ModbusRTU&) = delete;

  bool begin(const ModbusUartConfig& config);
  void task();
  void client() {
    if (!address_) {
      clientMode_ = true;
    }
  }
  bool server(uint8_t id) {
    if (clientMode_ || !id || id > 247) {
      return false;
    }
    address_ = id;
    return true;
  }
  [[nodiscard]] uint8_t server() const { return address_; }
  uint32_t eventSource() override { return source_; }

  bool Hregs(uint16_t offset, uint16_t* value, uint16_t registerCount = 1) { return this->Reg(HREG(offset), value, registerCount); }
  bool Coils(uint16_t offset, bool* value, uint16_t registerCount = 1) {
    if (!value || static_cast<uint32_t>(offset) + registerCount > 65536) {
      return false;
    }
    for (unsigned i = 0; i < registerCount; ++i) {
      if (!Coil(offset + i, value[i])) {
        return false;
      }
    }
    return true;
  }
  bool Istss(uint16_t offset, bool* value, uint16_t registerCount = 1) {
    if (!value || static_cast<uint32_t>(offset) + registerCount > 65536) {
      return false;
    }
    for (unsigned i = 0; i < registerCount; ++i) {
      if (!Ists(offset + i, value[i])) {
        return false;
      }
    }
    return true;
  }
  bool Iregs(uint16_t offset, uint16_t* value, uint16_t registerCount = 1) { return this->Reg(IREG(offset), value, registerCount); }

  bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t registerCount = 1);
  bool addCoil(uint16_t offset, bool value = false, uint16_t registerCount = 1);
  bool addIsts(uint16_t offset, bool value = false, uint16_t registerCount = 1);
  bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t registerCount = 1);

  bool Hreg(uint16_t offset, uint16_t value);
  bool Coil(uint16_t offset, bool value);
  bool Ists(uint16_t offset, bool value);
  bool Ireg(uint16_t offset, uint16_t value);

  bool Coil(uint16_t offset);
  bool Ists(uint16_t offset);
  uint16_t Ireg(uint16_t offset);
  uint16_t Hreg(uint16_t offset);

  bool removeCoil(uint16_t offset, uint16_t registerCount = 1);
  bool removeIsts(uint16_t offset, uint16_t registerCount = 1);
  bool removeIreg(uint16_t offset, uint16_t registerCount = 1);
  bool removeHreg(uint16_t offset, uint16_t registerCount = 1);

  bool onGetCoil(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSetCoil(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onGetHreg(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSetHreg(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onGetIsts(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSetIsts(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onGetIreg(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSetIreg(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);

  bool removeOnGetCoil(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSetCoil(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnGetHreg(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSetHreg(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnGetIsts(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSetIsts(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnGetIreg(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSetIreg(uint16_t offset, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);

  uint16_t writeCoil(uint8_t target, uint16_t offset, bool value, const cbTransaction& callbackFunction = nullptr);
  uint16_t writeCoil(uint8_t target, uint16_t offset, bool* value, uint16_t registerCount = 1, const cbTransaction& callbackFunction = nullptr);
  uint16_t readCoil(uint8_t target, uint16_t offset, bool* value, uint16_t registerCount = 1, const cbTransaction& callbackFunction = nullptr);
  uint16_t writeHreg(uint8_t target, uint16_t offset, uint16_t value, const cbTransaction& callbackFunction = nullptr);
  uint16_t writeHreg(uint8_t target, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, const cbTransaction& callbackFunction = nullptr);
  uint16_t readIsts(uint8_t target, uint16_t offset, bool* value, uint16_t registerCount = 1, const cbTransaction& callbackFunction = nullptr);
  uint16_t readHreg(uint8_t target, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, const cbTransaction& callbackFunction = nullptr);
  uint16_t readIreg(uint8_t target, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, const cbTransaction& callbackFunction = nullptr);

  uint16_t pushCoil(uint8_t target, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
  uint16_t pullCoil(uint8_t target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
  uint16_t pullIsts(uint8_t target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
  uint16_t pushHreg(uint8_t target, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
  uint16_t pullHreg(uint8_t target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
  uint16_t pullIreg(uint8_t target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);

  uint16_t pullHregToIreg(uint8_t target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
  uint16_t pullCoilToIsts(uint8_t target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
  uint16_t pushIstsToCoil(uint8_t target, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
  uint16_t pushIregToHreg(uint8_t target, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);

  uint16_t readFileRec(uint8_t slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction = nullptr);
  uint16_t writeFileRec(uint8_t slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction = nullptr);

  uint16_t maskHreg(uint8_t slaveId, uint16_t offset, uint16_t andMask, uint16_t orMask, cbTransaction callbackFunction = nullptr);
  uint16_t readWriteHreg(uint8_t target, uint16_t readOffset, uint16_t* readValue, uint16_t readRegisterCount, uint16_t writeOffset,
                         uint16_t* writeValue, uint16_t writeRegisterCount, cbTransaction callbackFunction = nullptr);

  uint16_t rawRequest(uint8_t target, const uint8_t* data, uint16_t len, cbTransaction callbackFunction = nullptr);
  uint16_t rawResponse(uint8_t target, const uint8_t* data, uint16_t len);
  uint16_t errorResponse(uint8_t target, Modbus::FunctionCode functionCode, Modbus::ResultCode exceptionCode);

  // Deprecated misspellings retained for source compatibility.
  uint16_t rawResponce(uint8_t target, uint8_t* data, uint16_t len) { return rawResponse(target, data, len); }
  uint16_t errorResponce(uint8_t target, Modbus::FunctionCode functionCode, Modbus::ResultCode exceptionCode) {
    return errorResponse(target, functionCode, exceptionCode);
  }

private:
  uint16_t send(uint8_t id, TAddress reg, cbTransaction callback, uint8_t* output = nullptr, bool waitResponse = true);

  void setDirection(bool transmitting) const;
  void transmit(uint8_t id);
  void processFrame();
  void finishTransaction(ResultCode result);

  uart_inst_t* uart_ = nullptr;
  int directionPin_ = -1;
  bool transmitLevel_ = true;
  int receiverEnablePin_ = -1;
  bool receiveLevel_ = false;
  uint32_t interFrameUs_ = 0;
  uint32_t interCharacterUs_ = 0;
  uint64_t lastActivity_ = 0;
  uint8_t rx_[MODBUS_MAX_FRAME + 3]{};
  size_t received_ = 0;
  bool discard_ = false;
  bool clientMode_ = false;
  uint8_t address_ = 0;
  uint8_t source_ = 0;
  uint64_t deadline_ = 0;
  cbTransaction callback_;
  uint8_t* output_ = nullptr;
  uint8_t request_[MODBUS_MAX_FRAME]{};
  size_t requestSize_ = 0;
  TAddress register_{};
};

#endif // MODBUS_RTU_PICO_H
