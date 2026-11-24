/*
    Modbus Library for Arduino
    Core functions
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2022 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once
#include "Arduino.h"
#include "ModbusSettings.h"
#if defined(MODBUS_USE_STL)
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
#else
#include "darray.h"
#endif

static inline uint16_t __swap_16(uint16_t num) { return (num >> 8) | (num << 8); }

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
#ifdef MODBUS_USE_STL
typedef std::function<uint16_t(TRegister* reg, uint16_t value)> cbModbus; // Callback function type
#else
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t value); // Callback function type
#endif

struct TAddress {
  enum RegType { COIL, ISTS, IREG, HREG };
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
  const TAddress operator+(const int& inc) const { // TAddress + integer
    TAddress result(*this);
    result.address += inc;
    return result;
  }
  bool isCoil() { return type == COIL; }
  bool isIsts() { return type == ISTS; }
  bool isIreg() { return type == IREG; }
  bool isHreg() { return type == HREG; }
};

struct TCallback {
  enum CallbackType { ON_SET, ON_GET };
  CallbackType type;
  TAddress address;
  cbModbus callback;
};

struct TRegister {
  TAddress address;
  uint16_t value;
  bool operator==(const TRegister& obj) const { return address == obj.address; }
};

class Modbus {
public:
  // Function Codes
  enum FunctionCode {
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
    FC_READWRITE_REGS = 0x17   // Read/Write Multiple registers
  };
  // Exception Codes
  // Custom result codes are used internally and by callbacks, but never in Modbus responses.
  enum ResultCode {
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
    EX_DATA_MISMACH = EX_DATA_MISMATCH, // Deprecated misspelling retained for source compatibility
    EX_UNEXPECTED_RESPONSE = 0xE3,      // Custom. Returned result does not match the transaction
    EX_TIMEOUT = 0xE4,                  // Custom. Operation not finished within reasonable time
    EX_CONNECTION_LOST = 0xE5,          // Custom. Connection with device lost
    EX_CANCEL = 0xE6,                   // Custom. Transaction/request canceled
    EX_PASSTHROUGH = 0xE7,              // Custom. Continue normal processing after the raw callback exits
    EX_FORCE_PROCESS = 0xE8             // Custom. Force processing after the raw callback exits
  };
  union RequestData {
    struct {
      TAddress reg;
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
    union {
      bool toServer;
      bool to_server; // Deprecated name retained for source compatibility.
    };
    union {
      uint8_t slaveId;
      struct {
        uint8_t unitId;
        union {
          uint32_t ipAddress;
          uint32_t ipaddr; // Deprecated name retained for source compatibility.
        };
        uint16_t transactionId;
      };
    };
    frame_arg_t(uint8_t serverId, bool isServerResponse = false) {
      slaveId = serverId;
      toServer = isServerResponse;
    };
    frame_arg_t(uint8_t unitIdentifier, uint32_t remoteAddress, uint16_t transactionIdentifier, bool isServerResponse = false) {
      unitId = unitIdentifier;
      ipAddress = remoteAddress;
      transactionId = transactionIdentifier;
      toServer = isServerResponse;
    };
  };

  ~Modbus();

  void cbEnable(bool state = true);
  void cbDisable();

private:
  ResultCode readBits(TAddress startRegister, uint16_t registerCount, FunctionCode functionCode);
  ResultCode readWords(TAddress startRegister, uint16_t registerCount, FunctionCode functionCode);

  bool setMultipleBits(uint8_t* frame, TAddress startRegister, uint16_t outputCount);
  bool setMultipleWords(uint16_t* frame, TAddress startRegister, uint16_t outputCount);

  void getMultipleBits(uint8_t* frame, TAddress startRegister, uint16_t registerCount);
  void getMultipleWords(uint16_t* frame, TAddress startRegister, uint16_t registerCount);

  void bitsToBool(bool* dst, uint8_t* src, uint16_t registerCount);
  void boolToBits(uint8_t* dst, bool* src, uint16_t registerCount);

protected:
  // Reply Types
  enum ReplyCode { REPLY_OFF = 0x01, REPLY_ECHO = 0x02, REPLY_NORMAL = 0x03, REPLY_ERROR = 0x04, REPLY_UNEXPECTED = 0x05 };
#if defined(MODBUS_USE_STL)
#if defined(MODBUS_GLOBAL_REGS)
  static std::vector<TRegister> _regs;
  static std::vector<TCallback> _callbacks;
#if defined(MODBUS_FILES)
  static std::function<ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> _onFile;
#endif
#else
  std::vector<TRegister> _regs;
  std::vector<TCallback> _callbacks;
#if defined(MODBUS_FILES)
  std::function<ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> _onFile;
#endif
#endif
#else
#if defined(MODBUS_GLOBAL_REGS)
  static DArray<TRegister, 1, 1> _regs;
  static DArray<TCallback, 1, 1> _callbacks;
#if defined(MODBUS_FILES)
  static ResultCode (*_onFile)(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*);
#endif
#else
  DArray<TRegister, 1, 1> _regs;
  DArray<TCallback, 1, 1> _callbacks;
#if defined(MODBUS_FILES)
  ResultCode (*_onFile)(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*) = nullptr;
#endif
#endif
#endif

  uint8_t* _frame = nullptr;
  uint16_t _len = 0;
  uint8_t _reply = 0;
  bool cbEnabled = true;
  uint16_t callback(TRegister* reg, uint16_t value, TCallback::CallbackType callbackType);
  virtual TRegister* searchRegister(TAddress addr);
  void exceptionResponse(FunctionCode functionCode, ResultCode exceptionCode);                    // Fills _frame with a response.
  void successResponse(TAddress startRegister, uint16_t outputCount, FunctionCode functionCode);  // Fills _frame with a response.
  void successResponce(TAddress startRegister, uint16_t outputCount, FunctionCode functionCode) { // Deprecated spelling.
    successResponse(startRegister, outputCount, functionCode);
  }
  void slavePDU(uint8_t* frame);                                                                           // Processes a server request.
  void masterPDU(uint8_t* frame, uint8_t* sourceFrame, TAddress startRegister, uint8_t* output = nullptr); // For Master
  // frame - data received from the server
  // sourceFrame - data sent to the server
  // startRegister - first local register in which to store data
  // output - if not null, store data in this buffer instead of local registers; the buffer must be an array of uint16_t or bool

  bool readSlave(uint16_t address, uint16_t registerCount, FunctionCode functionCode);
  bool writeSlaveBits(TAddress startRegister, uint16_t destination, uint16_t registerCount, FunctionCode functionCode, bool* data = nullptr);
  bool writeSlaveWords(TAddress startRegister, uint16_t destination, uint16_t registerCount, FunctionCode functionCode, uint16_t* data = nullptr);
  // startRegister - first local register from which to read data
  // destination - first server register to which to write data
  // registerCount - number of registers
  // functionCode - Modbus function code
  // data - if null, use local registers; otherwise, write data from the array to the server
  bool removeOn(TCallback::CallbackType callbackType, TAddress address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);

public:
  bool addReg(TAddress address, uint16_t value = 0, uint16_t registerCount = 1);
  bool Reg(TAddress address, uint16_t value);
  uint16_t Reg(TAddress address);
  bool removeReg(TAddress address, uint16_t registerCount = 1);
  bool addReg(TAddress address, uint16_t* value, uint16_t registerCount = 1);
  bool Reg(TAddress address, uint16_t* value, uint16_t registerCount = 1);

  bool onGet(TAddress address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSet(TAddress address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSet(TAddress address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnGet(TAddress address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);

  virtual uint32_t eventSource() { return 0; }
#if defined(MODBUS_USE_STL)
  typedef std::function<ResultCode(FunctionCode, const RequestData)> cbRequest; // Callback function Type
  typedef std::function<ResultCode(uint8_t*, uint8_t, void*)> cbRaw;            // Callback function Type
#else
  typedef ResultCode (*cbRequest)(FunctionCode functionCode, const RequestData data); // Callback function type
  typedef ResultCode (*cbRaw)(uint8_t*, uint8_t, void*);                              // Callback function type
#endif

protected:
  cbRaw _cbRaw = nullptr;
  static ResultCode _onRequestDefault(FunctionCode functionCode, const RequestData data);
  cbRequest _onRequest = _onRequestDefault;

public:
  bool onRaw(cbRaw callbackFunction = nullptr);
  bool onRequest(cbRequest callbackFunction = _onRequestDefault);
#if defined(MODBUSAPI_OPTIONAL)
protected:
  cbRequest _onRequestSuccess = _onRequestDefault;

public:
  bool onRequestSuccess(cbRequest callbackFunction = _onRequestDefault);
#endif

#if defined(MODBUS_FILES)
public:
#if defined(MODBUS_USE_STL)
  bool onFile(std::function<ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)>);
#else
  bool onFile(ResultCode (*callbackFunction)(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*));
#endif
private:
  ResultCode fileOp(FunctionCode functionCode, uint16_t fileNum, uint16_t recNum, uint16_t recLen, uint8_t* frame);

protected:
  bool readSlaveFile(uint16_t* fileNum, uint16_t* startRec, uint16_t* len, uint8_t count, FunctionCode functionCode);
  // fileNum - sequential array of file numbers to read
  // startRec - array of starting records for each file
  // len - array of counts of records to read in terms of register size (2 bytes) for each file
  // count - number of records to include in a single request
  // functionCode - Modbus function code; expected to be 0x14
  bool writeSlaveFile(uint16_t* fileNum, uint16_t* startRec, uint16_t* len, uint8_t count, FunctionCode functionCode, uint8_t* data);
  // fileNum - sequential array of file numbers to write
  // startRec - array of starting records for each file
  // len - array of counts of records to read in terms of register size (2 bytes) for each file
  // count - number of records to include in a single request
  // functionCode - Modbus function code; expected to be 0x15
  // data - sequential set of data records
#endif
};

#if defined(MODBUS_USE_STL)
typedef std::function<bool(Modbus::ResultCode, uint16_t, void*)> cbTransaction; // Callback skeleton for requests
#else
typedef bool (*cbTransaction)(Modbus::ResultCode event, uint16_t transactionId, void* data); // Callback skeleton for requests
#endif
// typedef Modbus::ResultCode (*cbRequest)(Modbus::FunctionCode func, TRegister* reg, uint16_t regCount); // Callback function Type
#if defined(MODBUS_FILES)
// Callback skeleton for file read/write
#if defined(MODBUS_USE_STL)
typedef std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> cbModbusFileOp;
#else
typedef Modbus::ResultCode (*cbModbusFileOp)(Modbus::FunctionCode functionCode, uint16_t fileNumber, uint16_t recordNumber, uint16_t recordLength,
                                             uint8_t* frame);
#endif
#endif

#if defined(ARDUINO_SAM_DUE_STL)
// Arduino Due STL workaround
namespace std {
void __throw_bad_function_call();
}
#endif
