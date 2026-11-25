/*
    Modbus RTU library
    Core functions
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2022 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#ifndef MODBUS_H
#define MODBUS_H

#include "ModbusSettings.h"
#include <functional>
#include <optional>
#include <vector>

namespace modbus_detail {
// NOLINTNEXTLINE(bugprone-signed-bitwise)
inline uint16_t readWord(const uint8_t* bytes) { return (static_cast<uint16_t>(bytes[0]) << 8U) | bytes[1]; }
inline void writeWord(uint8_t* bytes, uint16_t value) {
  bytes[0] = static_cast<uint8_t>(value >> 8U);
  bytes[1] = static_cast<uint8_t>(value);
}
} // namespace modbus_detail

#define COIL(addressValue)                                                                                                                           \
  (TAddress) { TAddress::COIL, addressValue }
#define ISTS(addressValue)                                                                                                                           \
  (TAddress) { TAddress::ISTS, addressValue }
#define IREG(addressValue)                                                                                                                           \
  (TAddress) { TAddress::IREG, addressValue }
#define HREG(addressValue)                                                                                                                           \
  (TAddress) { TAddress::HREG, addressValue }
#define BIT_VAL(bitValue)     ((bitValue) ? 0xFF00 : 0x0000)
#define BIT_BOOL(bitValue)    ((bitValue) == 0xFF00)
#define COIL_VAL(coilValue)   ((coilValue) ? 0xFF00 : 0x0000)
#define COIL_BOOL(coilValue)  ((coilValue) == 0xFF00)
#define ISTS_VAL(inputValue)  ((inputValue) ? 0xFF00 : 0x0000)
#define ISTS_BOOL(inputValue) ((inputValue) == 0xFF00)

// Retained for compatibility with the deprecated v1.x onSet/onGet format.
#define cbDefault nullptr

struct TRegister;
using cbModbus = std::function<uint16_t(TRegister* reg, uint16_t value)>; // Callback function type

struct TAddress {
  // NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
  enum RegType : uint8_t { COIL, ISTS, IREG, HREG };
  RegType type;
  uint16_t address;
  bool operator==(const TAddress& obj) const { // TAddress == TAddress
    return type == obj.type && address == obj.address;
  }
  bool operator!=(const TAddress& obj) const { // TAddress != TAddress
    return type != obj.type || address != obj.address;
  }
  TAddress& operator++() { // ++TAddress
    address++;
    return *this;
  }
  TAddress operator++(int) { // TAddress++
    TAddress result(*this);
    ++(*this);
    return result;
  }
  TAddress& operator+=(const int& inc) { // TAddress += integer
    address += inc;
    return *this;
  }
  TAddress operator+(const int& inc) const { // TAddress + integer
    TAddress result(*this);
    result.address += inc;
    return result;
  }
  [[nodiscard]] bool isCoil() const { return type == COIL; }
  [[nodiscard]] bool isIsts() const { return type == ISTS; }
  [[nodiscard]] bool isIreg() const { return type == IREG; }
  [[nodiscard]] bool isHreg() const { return type == HREG; }
};

struct TCallback {
  // NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
  enum CallbackType : uint8_t { ON_SET, ON_GET };
  struct Id {
    uint32_t value{};
    bool operator==(Id other) const { return value == other.value; }
  };
  CallbackType type;
  TAddress address;
  cbModbus callback;
  Id id{};
};

struct TRegister {
  TAddress address;
  uint16_t value;
  bool operator==(const TRegister& obj) const { return address == obj.address; }
};

class Modbus {
public:
  using CallbackId = TCallback::Id;

  // Function Codes
  // NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
  enum FunctionCode : uint8_t {
    FC_READ_COILS = 0x01,      // Read Coils (Output) Status
    FC_READ_INPUT_STAT = 0x02, // Read Input Status (Discrete Inputs)
    FC_READ_REGS = 0x03,       // Read Holding Registers
    FC_READ_INPUT_REGS = 0x04, // Read Input Registers
    FC_WRITE_COIL = 0x05,      // Write Single Coil (Output)
    FC_WRITE_REG = 0x06,       // Preset Single Register
    FC_DIAGNOSTICS = 0x08,     // Not implemented. Diagnostics (Serial Line only)
    FC_WRITE_COILS = 0x0F,     // Write Multiple Coils (Outputs)
    FC_WRITE_REGS = 0x10,      // Write block of contiguous registers
    FC_READ_FILE_REC = 0x14,   // Read File Record
    FC_WRITE_FILE_REC = 0x15,  // Write File Record
    FC_MASKWRITE_REG = 0x16,   // Mask Write Register
    FC_READWRITE_REGS = 0x17,  // Read/Write Multiple registers
  };
  // Exception Codes
  // Custom result codes are used internally and by callbacks, but never in Modbus responses.
  // NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
  enum ResultCode : uint8_t {
    EX_SUCCESS = 0x00,                  // Custom. No error
    EX_ILLEGAL_FUNCTION = 0x01,         // Function code is not supported
    EX_ILLEGAL_ADDRESS = 0x02,          // Output address does not exist
    EX_ILLEGAL_VALUE = 0x03,            // Output value is out of range
    EX_SLAVE_FAILURE = 0x04,            // Server or client device failed to process the request
    EX_ACKNOWLEDGE = 0x05,              // Not used
    EX_SLAVE_DEVICE_BUSY = 0x06,        // Not used
    EX_MEMORY_PARITY_ERROR = 0x08,      // Not used
    EX_PATH_UNAVAILABLE = 0x0A,         // Not used
    EX_DEVICE_FAILED_TO_RESPOND = 0x0B, // Not used
    EX_GENERAL_FAILURE = 0xE1,          // Custom. Unexpected master error
    EX_DATA_MISMATCH = 0xE2,            // Custom. Input data size mismatch
    EX_UNEXPECTED_RESPONSE = 0xE3,      // Custom. Returned result does not match the transaction
    EX_TIMEOUT = 0xE4,                  // Custom. Operation not finished within reasonable time
    EX_CONNECTION_LOST = 0xE5,          // Custom. Connection with device lost
    EX_CANCEL = 0xE6,                   // Custom. Transaction/request canceled
    EX_PASSTHROUGH = 0xE7,              // Custom. Continue normal processing after the raw callback exits
    EX_FORCE_PROCESS = 0xE8,            // Custom. Force processing after the raw callback exits
  };
  union RequestData {
    struct {
      TAddress reg{};
      uint16_t regCount;
    };
    struct {
      TAddress regRead;
      uint16_t regReadCount;
      TAddress regWrite;
      uint16_t regWriteCount;
    };
    struct {
      TAddress regMask;
      uint16_t andMask;
      uint16_t orMask;
    };
    RequestData(TAddress registerAddress, uint16_t registerCount) {
      reg = registerAddress;
      regCount = registerCount;
    };
    RequestData(TAddress readAddress, uint16_t readCount, TAddress writeAddress, uint16_t writeCount) {
      regRead = readAddress;
      regReadCount = readCount;
      regWrite = writeAddress;
      regWriteCount = writeCount;
    };
    RequestData(TAddress maskAddress, uint16_t andMaskValue, uint16_t orMaskValue) {
      regMask = maskAddress;
      andMask = andMaskValue;
      orMask = orMaskValue;
    };
  };

  struct frame_arg_t {
    uint8_t slaveId;
    bool toServer;
  };

  virtual ~Modbus() = default;

  void cbEnable(bool state = true);
  void cbDisable();

private:
  friend class ModbusRTU;

  std::vector<TRegister>::iterator registerPosition(TAddress address);
  ResultCode readBits(TAddress startRegister, uint16_t registerCount, FunctionCode functionCode);
  ResultCode readWords(TAddress startRegister, uint16_t registerCount, FunctionCode functionCode);

  bool setMultipleBits(const uint8_t* frame, TAddress startRegister, uint16_t outputCount);
  bool setMultipleWords(const uint8_t* frame, TAddress startRegister, uint16_t registerCount);

  void getMultipleBits(uint8_t* frame, TAddress startRegister, uint16_t registerCount);
  void getMultipleWords(uint8_t* frame, TAddress startRegister, uint16_t registerCount);

  static void bitsToBool(bool* dst, const uint8_t* src, uint16_t registerCount);
  static void boolToBits(uint8_t* dst, const bool* src, uint16_t registerCount);

  // Reply Types
  // NOLINTNEXTLINE(cppcoreguidelines-use-enum-class)
  enum ReplyCode : uint8_t { REPLY_OFF = 0x01, REPLY_ECHO = 0x02, REPLY_NORMAL = 0x03, REPLY_ERROR = 0x04, REPLY_UNEXPECTED = 0x05 };
  std::vector<TRegister> _regs;
  std::vector<TCallback> _callbacks;
  uint32_t _nextCallbackId = 1;
  std::function<ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> _onFile;

  uint8_t _frame[MODBUS_MAX_FRAME]{};
  uint16_t _len = 0;
  uint8_t _reply = 0;
  bool cbEnabled = true;
  uint16_t callback(TRegister* reg, uint16_t value, TCallback::CallbackType callbackType);
  bool addOn(TCallback::CallbackType callbackType, TAddress address, const cbModbus& callbackFunction, uint16_t registerCount, CallbackId id = {});
  std::optional<CallbackId>
  addOnWithId(TCallback::CallbackType callbackType, TAddress address, const cbModbus& callbackFunction, uint16_t registerCount);
  bool removeOn(TCallback::CallbackType callbackType, TAddress address, const cbModbus& callbackFunction, uint16_t registerCount);
  virtual TRegister* searchRegister(TAddress addr);
  void exceptionResponse(FunctionCode functionCode, ResultCode exceptionCode);                    // Fills _frame with a response.
  void successResponse(TAddress startRegister, uint16_t outputCount, FunctionCode functionCode);  // Fills _frame with a response.
  void successResponce(TAddress startRegister, uint16_t outputCount, FunctionCode functionCode) { // Deprecated spelling.
    successResponse(startRegister, outputCount, functionCode);
  }
  void slavePDU(uint8_t* frame);                                                                                 // Processes a server request.
  void masterPDU(uint8_t* frame, const uint8_t* sourceFrame, TAddress startRegister, uint8_t* output = nullptr); // For Master
  // frame - data received from the server
  // sourceFrame - data sent to the server
  // startRegister - first local register in which to store data
  // output - if not null, store data in this buffer instead of local registers; the buffer must be an array of uint16_t or bool

public:
  bool readSlave(uint16_t address, uint16_t registerCount, FunctionCode functionCode);
  bool writeSlaveBits(TAddress startRegister, uint16_t destination, uint16_t registerCount, FunctionCode functionCode, bool* data = nullptr);
  bool writeSlaveWords(TAddress startRegister, uint16_t destination, uint16_t registerCount, FunctionCode functionCode, uint16_t* data = nullptr);
  // startRegister - first local register from which to read data
  // destination - first server register to which to write data
  // registerCount - number of registers
  // functionCode - Modbus function code
  // data - if null, use local registers; otherwise, write data from the array to the server

  bool addReg(TAddress address, uint16_t value = 0, uint16_t registerCount = 1);
  bool Reg(TAddress address, uint16_t value);
  uint16_t Reg(TAddress address);
  bool removeReg(TAddress address, uint16_t registerCount = 1);
  bool addReg(TAddress address, uint16_t* value, uint16_t registerCount = 1);
  bool Reg(TAddress address, uint16_t* value, uint16_t registerCount = 1);

  bool onGet(TAddress address, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSet(TAddress address, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  // A registration ID belongs to this instance and covers the entire registration.
  [[nodiscard]] std::optional<CallbackId> onGetWithId(TAddress address, const cbModbus& callbackFunction, uint16_t registerCount = 1);
  [[nodiscard]] std::optional<CallbackId> onSetWithId(TAddress address, const cbModbus& callbackFunction, uint16_t registerCount = 1);
  bool removeCallback(CallbackId id);
  // Pass nullptr to remove by address. Use removeCallback for a specific registration.
  bool removeOnSet(TAddress address, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnGet(TAddress address, const cbModbus& callbackFunction = nullptr, uint16_t registerCount = 1);

  virtual uint32_t eventSource() { return 0; }
  using cbRequest = std::function<ResultCode(FunctionCode, const RequestData)>; // Callback function Type
  using cbRaw = std::function<ResultCode(uint8_t*, uint8_t, void*)>;            // Callback function Type

private:
  cbRaw _cbRaw = nullptr;
  static ResultCode _onRequestDefault(FunctionCode functionCode, RequestData data);
  cbRequest _onRequest = _onRequestDefault;

public:
  bool onRaw(const cbRaw& callbackFunction = nullptr);
  bool onRequest(const cbRequest& callbackFunction = _onRequestDefault);

private:
  cbRequest _onRequestSuccess = _onRequestDefault;

public:
  bool onRequestSuccess(const cbRequest& callbackFunction = _onRequestDefault);
  bool onFile(const std::function<ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)>& callbackFunction);

private:
  ResultCode fileOp(FunctionCode functionCode, uint16_t fileNum, uint16_t recNum, uint16_t recLen, uint8_t* frame);

protected:
  bool readSlaveFile(const uint16_t* fileNum, const uint16_t* startRec, const uint16_t* len, uint8_t count, FunctionCode functionCode);
  // fileNum - sequential array of file numbers to read
  // startRec - array of starting records for each file
  // len - array of counts of records to read in terms of register size (2 bytes) for each file
  // count - number of records to include in a single request
  // functionCode - Modbus function code; expected to be 0x14
  bool
  writeSlaveFile(const uint16_t* fileNum, const uint16_t* startRec, const uint16_t* len, uint8_t count, FunctionCode functionCode, uint8_t* data);
  // fileNum - sequential array of file numbers to write
  // startRec - array of starting records for each file
  // len - array of counts of records to read in terms of register size (2 bytes) for each file
  // count - number of records to include in a single request
  // functionCode - Modbus function code; expected to be 0x15
  // data - sequential set of data records
};

using cbTransaction = std::function<bool(Modbus::ResultCode, uint16_t, void*)>; // Callback skeleton for requests
// typedef Modbus::ResultCode (*cbRequest)(Modbus::FunctionCode func, TRegister* reg, uint16_t regCount); // Callback function Type
// Callback skeleton for file read/write
using cbModbusFileOp = std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)>;

#endif // MODBUS_H
