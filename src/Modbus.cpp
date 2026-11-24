/*
    Modbus Library for Arduino
    Core functions
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2021 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "Modbus.h"

#if defined(MODBUS_GLOBAL_REGS)
#if defined(MODBUS_USE_STL)
std::vector<TRegister> Modbus::_regs;
std::vector<TCallback> Modbus::_callbacks;
#ifdef MODBUS_FILES
std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> Modbus::_onFile;
#endif
#else
DArray<TRegister, 1, 1> Modbus::_regs;
DArray<TCallback, 1, 1> Modbus::_callbacks;
#if defined(MODBUS_FILES)
cbModbusFileOp Modbus::_onFile = nullptr;
#endif
#endif
#endif

uint16_t Modbus::callback(TRegister* reg, uint16_t value, TCallback::CallbackType callbackType) {
#define MODBUS_COMPARE_CB                                                                                                                            \
  [reg, callbackType](TCallback& callbackEntry) { return callbackEntry.address == reg->address && callbackEntry.type == callbackType; }
  uint16_t newValue = value;
#ifdef MODBUS_USE_STL
  std::vector<TCallback>::iterator iterator = _callbacks.begin();
  do {
    iterator = std::find_if(iterator, _callbacks.end(), MODBUS_COMPARE_CB);
    if (iterator != _callbacks.end()) {
      newValue = iterator->callback(reg, newValue);
      iterator++;
    }
  } while (iterator != _callbacks.end());
#else
  size_t callbackIndex = 0;
  do {
    callbackIndex = _callbacks.find(MODBUS_COMPARE_CB, callbackIndex);
    if (callbackIndex < _callbacks.size()) newValue = _callbacks[callbackIndex].callback(reg, newValue);
    callbackIndex++;
  } while (callbackIndex < _callbacks.size());
#endif
  return newValue;
}

TRegister* Modbus::searchRegister(TAddress address) {
#define MODBUS_COMPARE_REG [address](TRegister& registerEntry) { return registerEntry.address == address; }
#ifdef MODBUS_USE_STL
  std::vector<TRegister>::iterator iterator = std::find_if(_regs.begin(), _regs.end(), MODBUS_COMPARE_REG);
  if (iterator != _regs.end()) return &*iterator;
#else
  size_t registerIndex = _regs.find(MODBUS_COMPARE_REG);
  if (registerIndex < _regs.size()) return _regs.entry(registerIndex);
#endif
  return nullptr;
}

bool Modbus::addReg(TAddress address, uint16_t value, uint16_t registerCount) {
#ifdef MODBUS_MAX_REGS
  if (_regs.size() + registerCount > MODBUS_MAX_REGS) return false;
#endif
  if (0xFFFF - address.address < registerCount) registerCount = 0xFFFF - address.address;
  for (uint16_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
    if (!searchRegister(address + registerIndex)) _regs.push_back({ address + registerIndex, value });
  }
  // std::sort(_regs.begin(), _regs.end());
  return true;
}

bool Modbus::Reg(TAddress address, uint16_t value) {
  TRegister* reg;
  reg = searchRegister(address); // Search for the register address.
  if (reg) {                     // If found, assign the new register value.
    if (cbEnabled) {
      reg->value = callback(reg, value, TCallback::ON_SET);
    } else {
      reg->value = value;
    }
    return true;
  } else return false;
}

uint16_t Modbus::Reg(TAddress address) {
  TRegister* reg;
  reg = searchRegister(address);
  if (reg)
    if (cbEnabled) {
      return callback(reg, reg->value, TCallback::ON_GET);
    } else {
      return reg->value;
    }
  else return 0;
}

bool Modbus::removeReg(TAddress address, uint16_t registerCount) {
  TRegister* reg;
  bool atLeastOne = false;
  if (0xFFFF - address.address < registerCount) registerCount = 0xFFFF - address.address;
  for (uint16_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
    reg = searchRegister(address + registerIndex);
    if (reg) {
      atLeastOne = true;
      removeOnSet(address + registerIndex);
      removeOnGet(address + registerIndex);
#if defined(MODBUS_USE_STL)
      _regs.erase(std::remove(_regs.begin(), _regs.end(), *reg), _regs.end());
#else
      _regs.remove(_regs.find(MODBUS_COMPARE_REG));
#endif
    }
  }
  return atLeastOne;
}

bool Modbus::addReg(TAddress address, uint16_t* value, uint16_t registerCount) {
  if (0xFFFF - address.address < registerCount) registerCount = 0xFFFF - address.address;
  for (uint16_t registerIndex = 0; registerIndex < registerCount; registerIndex++) addReg(address + registerIndex, value[registerIndex]);
  return true;
}

void Modbus::slavePDU(uint8_t* frame) {
  FunctionCode fcode = (FunctionCode)frame[0];
  uint16_t field1 = (uint16_t)frame[1] << 8 | (uint16_t)frame[2];
  uint16_t field2 = (uint16_t)frame[3] << 8 | (uint16_t)frame[4];
  uint16_t field3 = 0;
  uint16_t field4 = 0;
  uint16_t calculatedByteCount;
  uint16_t registerIndex;
  ResultCode resultCode;
  switch (fcode) {
    case FC_WRITE_REG:
      // field1 = reg, field2 = value
      resultCode = _onRequest(fcode, { HREG(field1), field2 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      if (!Reg(HREG(field1), field2)) { // Check Address and execute (reg exists?)
        exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
        return;
      }
      if (Reg(HREG(field1)) != field2) { // Check for failure
        exceptionResponse(fcode, EX_SLAVE_FAILURE);
        return;
      }
      _reply = REPLY_ECHO;
      _onRequestSuccess(fcode, { HREG(field1), field2 });
      break;

    case FC_READ_REGS:
      // field1 = startRegister, field2 = registerCount, header length = 3
      resultCode = _onRequest(fcode, { HREG(field1), field2 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      resultCode = readWords(HREG(field1), field2, fcode);
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      _onRequestSuccess(fcode, { HREG(field1), field2 });
      break;

    case FC_WRITE_REGS:
      // field1 = startRegister, field2 = registerCount, frame[5] = data length, header length = 6
      resultCode = _onRequest(fcode, { HREG(field1), field2 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      if (field2 < 0x0001 || field2 > MODBUS_MAX_WORDS || 0xFFFF - field1 < field2 || frame[5] != 2 * field2) { // Check constraints.
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      for (registerIndex = 0; registerIndex < field2; registerIndex++) { // Check Address (startRegister...startRegister + registerCount)
        if (!searchRegister(HREG(field1) + registerIndex)) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
      }
      if (!setMultipleWords((uint16_t*)(frame + 6), HREG(field1), field2)) {
        exceptionResponse(fcode, EX_SLAVE_FAILURE);
        return;
      }
      successResponse(HREG(field1), field2, fcode);
      _reply = REPLY_NORMAL;
      _onRequestSuccess(fcode, { HREG(field1), field2 });
      break;

    case FC_READ_COILS:
      // field1 = startRegister, field2 = registerCount
      resultCode = _onRequest(fcode, { COIL(field1), field2 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      resultCode = readBits(COIL(field1), field2, fcode);
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      _onRequestSuccess(fcode, { COIL(field1), field2 });
      break;

    case FC_READ_INPUT_STAT:
      // field1 = startRegister, field2 = registerCount
      resultCode = _onRequest(fcode, { ISTS(field1), field2 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      resultCode = readBits(ISTS(field1), field2, fcode);
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      _onRequestSuccess(fcode, { ISTS(field1), field2 });
      break;

    case FC_READ_INPUT_REGS:
      // field1 = startRegister, field2 = registerCount
      resultCode = _onRequest(fcode, { IREG(field1), field2 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      resultCode = readWords(IREG(field1), field2, fcode);
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      _onRequestSuccess(fcode, { IREG(field1), field2 });
      break;

    case FC_WRITE_COIL:
      // field1 = register address, field2 = status, header length = 3
      resultCode = _onRequest(fcode, { COIL(field1), field2 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      if (field2 != 0xFF00 && field2 != 0x0000) { // Check value (status)
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      if (!Reg(COIL(field1), field2)) { // Check Address and execute (reg exists?)
        exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
        return;
      }
      if (Reg(COIL(field1)) != field2) { // Check for failure
        exceptionResponse(fcode, EX_SLAVE_FAILURE);
        return;
      }
      _reply = REPLY_ECHO;
      _onRequestSuccess(fcode, { COIL(field1), field2 });
      break;

    case FC_WRITE_COILS:
      // field1 = startRegister, field2 = registerCount, frame[5] = byte count, header length = 6
      resultCode = _onRequest(fcode, { COIL(field1), field2 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      calculatedByteCount = field2 / 8;
      if (field2 % 8) calculatedByteCount++;
      if (field2 < 0x0001 || field2 > MODBUS_MAX_BITS || 0xFFFF - field1 < field2 ||
          frame[5] != calculatedByteCount) { // Check the register range and data size.
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      for (registerIndex = 0; registerIndex < field2; registerIndex++) { // Check Address (startRegister...startRegister + registerCount)
        if (!searchRegister(COIL(field1) + registerIndex)) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
      }
      if (!setMultipleBits(frame + 6, COIL(field1), field2)) {
        exceptionResponse(fcode, EX_SLAVE_FAILURE);
        return;
      }
      successResponse(COIL(field1), field2, fcode);
      _reply = REPLY_NORMAL;
      _onRequestSuccess(fcode, { COIL(field1), field2 });
      break;
#if defined(MODBUS_FILES)
    case FC_READ_FILE_REC:
      if (frame[1] < 0x07 || frame[1] > 0xF5) { // Wrong request data size
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      {
        uint8_t bufSize = 2;                                                    // 2 bytes for frame header
        uint8_t* recs = frame + 2;                                              // Begin of sub-recs blocks
        uint8_t recsCount = frame[1] / 7;                                       // Number of subrequest blocks.
        for (uint8_t recordIndex = 0; recordIndex < recsCount; recordIndex++) { // Calculate the required output-buffer size.
          // uint16_t fileNum = (uint16_t)recs[1] << 8 | (uint16_t)recs[2];
          uint16_t recNum = (uint16_t)recs[3] << 8 | (uint16_t)recs[4];
          uint16_t recLen = (uint16_t)recs[5] << 8 | (uint16_t)recs[6];
          // Serial.printf("%d, %d, %d\n", fileNum, recNum, recLen);
          if (recs[0] != 0x06 || recNum > 0x270F) { // Invalid reference type or record count.
            exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
            return;
          }
          bufSize += recLen * 2 + 2; // 4 bytes for header + data
          recs += 7;
        }
        if (bufSize > MODBUS_MAX_FRAME) { // Frame to return too large
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
        uint8_t* srcFrame = _frame;
        _frame = (uint8_t*)malloc(bufSize);
        if (!_frame) {
          free(srcFrame);
          exceptionResponse(fcode, EX_SLAVE_FAILURE);
          return;
        }
        _len = bufSize;
        recs = frame + 2; // Begin of sub-recs blocks
        uint8_t* data = _frame + 2;
        for (uint8_t recordIndex = 0; recordIndex < recsCount; recordIndex++) {
          uint16_t fileNum = (uint16_t)recs[1] << 8 | (uint16_t)recs[2];
          uint16_t recNum = (uint16_t)recs[3] << 8 | (uint16_t)recs[4];
          uint16_t recLen = (uint16_t)recs[5] << 8 | (uint16_t)recs[6];
          ResultCode res = fileOp(fcode, fileNum, recNum, recLen, data + 2);
          if (res != EX_SUCCESS) { // File read failed
            free(srcFrame);
            exceptionResponse(fcode, res);
            return;
          }
          data[0] = recLen * 2 + 1;
          data[1] = 0x06;
          data += recLen * 2 + 2;
          recs += 7;
        }
        _frame[0] = fcode;
        _frame[1] = bufSize;
        _reply = REPLY_NORMAL;
        free(srcFrame);
      }
      break;
    case FC_WRITE_FILE_REC: {
      if (frame[1] < 0x09 || frame[1] > 0xFB) { // Wrong request data size
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      uint8_t* recs = frame + 2; // Begin of sub-recs blocks
      while (recs < frame + frame[1]) {
        if (recs[0] != 0x06) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
        uint16_t fileNum = (uint16_t)recs[1] << 8 | (uint16_t)recs[2];
        uint16_t recNum = (uint16_t)recs[3] << 8 | (uint16_t)recs[4];
        uint16_t recLen = (uint16_t)recs[5] << 8 | (uint16_t)recs[6];
        if (recs + recLen * 2 > frame + frame[1]) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
        ResultCode res = fileOp(fcode, fileNum, recNum, recLen, recs + 7);
        if (res != EX_SUCCESS) { // File write failed
          exceptionResponse(fcode, res);
          return;
        }
        recs += 7 + recLen * 2;
      }
    }
      _reply = REPLY_ECHO;
      break;
#endif
    case FC_MASKWRITE_REG:
      // field1 = reg, field2 = AND mask, field3 = OR mask
      //  Result = (Current Contents AND And_Mask) OR (Or_Mask AND (NOT And_Mask))
      field3 = (uint16_t)frame[5] << 8 | (uint16_t)frame[6];
      resultCode = _onRequest(fcode, { HREG(field1), field2, field3 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      field4 = Reg(HREG(field1));
      field4 = (field4 & field2) | (field3 & ~field2);
      if (!Reg(HREG(field1), field4)) { // Check Address and execute (reg exists?)
        exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
        return;
      }
      if (Reg(HREG(field1)) != field4) { // Check for failure
        exceptionResponse(fcode, EX_SLAVE_FAILURE);
        return;
      }
      _reply = REPLY_ECHO;
      _onRequestSuccess(fcode, { HREG(field1), field2, field3 });
      break;
    case FC_READWRITE_REGS:
      // field1 = first register to read, field2 = read count, frame[9] = data length, header length = 10
      // field3 = first register to write, field4 = write count
      field3 = (uint16_t)frame[5] << 8 | (uint16_t)frame[6];
      field4 = (uint16_t)frame[7] << 8 | (uint16_t)frame[8];
      resultCode = _onRequest(fcode, { HREG(field1), field2, HREG(field3), field4 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      if (field2 < 0x0001 || field2 > MODBUS_MAX_WORDS || field4 < 0x0001 || field4 > MODBUS_MAX_WORDS || 0xFFFF - field1 < field2 ||
          0xFFFF - field1 < field2 || frame[9] != 2 * field4) { // Check value
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      if (!setMultipleWords((uint16_t*)(frame + 10), HREG(field3), field4)) {
        exceptionResponse(fcode, EX_SLAVE_FAILURE);
        return;
      }
      resultCode = readWords(HREG(field1), field2, fcode);
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      _onRequestSuccess(fcode, { HREG(field1), field2, HREG(field3), field4 });
      break;

    default: exceptionResponse(fcode, EX_ILLEGAL_FUNCTION); return;
  }
}

void Modbus::successResponse(TAddress startRegister, uint16_t outputCount, FunctionCode functionCode) {
  free(_frame);
  _len = 5;
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) {
    _reply = REPLY_OFF;
    return;
  }
  _frame[0] = functionCode;
  _frame[1] = startRegister.address >> 8;
  _frame[2] = startRegister.address & 0x00FF;
  _frame[3] = outputCount >> 8;
  _frame[4] = outputCount & 0x00FF;
}

void Modbus::exceptionResponse(FunctionCode functionCode, ResultCode exceptionCode) {
  free(_frame);
  _len = 2;
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) {
    _reply = REPLY_OFF;
    return;
  }
  _frame[0] = functionCode + 0x80;
  _frame[1] = exceptionCode;
  _reply = REPLY_NORMAL;
}

void Modbus::getMultipleBits(uint8_t* frame, TAddress startRegister, uint16_t registerCount) {
  uint8_t bitIndex = 0;
  uint16_t byteIndex = 0;
  while (registerCount--) {
    if (BIT_BOOL(Reg(startRegister))) bitSet(frame[byteIndex], bitIndex);
    else bitClear(frame[byteIndex], bitIndex);
    bitIndex++;
    if (bitIndex == 8) {
      byteIndex++;
      bitIndex = 0;
    }
    startRegister++; // increment the register
  }
}

void Modbus::getMultipleWords(uint16_t* frame, TAddress startRegister, uint16_t registerCount) {
  for (uint8_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
    frame[registerIndex] = __swap_16(Reg(startRegister + registerIndex));
  }
}

Modbus::ResultCode Modbus::readBits(TAddress startRegister, uint16_t registerCount, FunctionCode functionCode) {
  if (registerCount < 0x0001 || registerCount > MODBUS_MAX_BITS || (0xFFFF - startRegister.address) < registerCount) return EX_ILLEGAL_VALUE;
  // Check the address.
  // Check only startRegister. Is this correct?
  // Checking every register in the range causes errors in ScadaBR.
  // ScadaBR requests multiple registers in one request when more than one data point of the same type is configured.
#if defined(MODBUS_STRICT_REG)
  for (uint16_t registerIndex = 0; registerIndex < registerCount;
       registerIndex++) { // Check addresses from startRegister through startRegister + registerCount.
    if (!searchRegister(startRegister + registerIndex)) return EX_ILLEGAL_VALUE;
  }
#else
  if (!searchRegister(startRegister)) return EX_ILLEGAL_VALUE;
#endif
  free(_frame);
  // Determine the message length = function type, byte count and
  // for each group of 8 registers the message length increases by 1
  _len = 2 + registerCount / 8;
  if (registerCount % 8) _len++; // Add 1 to the message length for the partial byte.
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) return EX_SLAVE_FAILURE;
  _frame[0] = functionCode;
  _frame[1] = _len - 2; // byte count (_len - function code and byte count)
  _frame[_len - 1] = 0; // Clean last probably partial byte
  getMultipleBits(_frame + 2, startRegister, registerCount);
  _reply = REPLY_NORMAL;
  return EX_SUCCESS;
}

Modbus::ResultCode Modbus::readWords(TAddress startRegister, uint16_t registerCount, FunctionCode functionCode) {
  // Check value (registerCount)
  if (registerCount < 0x0001 || registerCount > MODBUS_MAX_WORDS || 0xFFFF - startRegister.address < registerCount) return EX_ILLEGAL_VALUE;
#if defined(MODBUS_STRICT_REG)
  for (uint16_t registerIndex = 0; registerIndex < registerCount;
       registerIndex++) { // Check addresses from startRegister through startRegister + registerCount.
    if (!searchRegister(startRegister + registerIndex)) return EX_ILLEGAL_VALUE;
  }
#else
  if (!searchRegister(startRegister)) return EX_ILLEGAL_ADDRESS;
#endif
  free(_frame);
  _len = 2 + registerCount * 2; // Two bytes per register plus two header bytes.
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) return EX_SLAVE_FAILURE;
  _frame[0] = functionCode;
  _frame[1] = _len - 2; // byte count
  getMultipleWords((uint16_t*)(_frame + 2), startRegister, registerCount);
  _reply = REPLY_NORMAL;
  return EX_SUCCESS;
}

bool Modbus::setMultipleBits(uint8_t* frame, TAddress startRegister, uint16_t outputCount) {
  uint8_t bitIndex = 0;
  uint16_t byteIndex = 0;
  bool result = true;
  while (outputCount--) {
    Reg(startRegister, BIT_VAL(bitRead(frame[byteIndex], bitIndex)));
    if (Reg(startRegister) != BIT_VAL(bitRead(frame[byteIndex], bitIndex))) result = false;
    bitIndex++;
    if (bitIndex == 8) {
      byteIndex++;
      bitIndex = 0;
    }
    startRegister++; // increment the register
  }
  return result;
}

bool Modbus::setMultipleWords(uint16_t* frame, TAddress startRegister, uint16_t registerCount) {
  bool result = true;
  for (uint8_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
    Reg(startRegister + registerIndex, __swap_16(frame[registerIndex]));
    if (Reg(startRegister + registerIndex) != __swap_16(frame[registerIndex])) result = false;
  }
  return result;
}

bool Modbus::onGet(TAddress address, cbModbus callbackFunction, uint16_t registerCount) {
  TRegister* reg;
  bool atLeastOne = false;
  if (!callbackFunction) {
    return removeOnGet(address);
  }
  while (registerCount > 0) {
    reg = searchRegister(address);
    if (reg) {
      _callbacks.push_back({ TCallback::ON_GET, address, callbackFunction });
      atLeastOne = true;
    }
    address++;
    registerCount--;
  }
  return atLeastOne;
}
bool Modbus::onSet(TAddress address, cbModbus callbackFunction, uint16_t registerCount) {
  TRegister* reg;
  bool atLeastOne = false;
  if (!callbackFunction) {
    return removeOnSet(address);
  }
  while (registerCount > 0) {
    reg = searchRegister(address);
    if (reg) {
      _callbacks.push_back({ TCallback::ON_SET, address, callbackFunction });
      atLeastOne = true;
    }
    address++;
    registerCount--;
  }
  return atLeastOne;
}

bool Modbus::removeOn(TCallback::CallbackType callbackType, TAddress address, cbModbus callbackFunction, uint16_t registerCount) {
  size_t originalSize = _callbacks.size();
#if defined(MODBUS_USE_STL)
#define MODBUS_COMPARE_ON                                                                                                                            \
  [callbackType, address, callbackFunction](const TCallback entry) {                                                                                 \
    return entry.type == callbackType && entry.address == address &&                                                                                 \
           (!callbackFunction || std::addressof(callbackFunction) == std::addressof(entry.callback));                                                \
  }
  while (registerCount--) {
    _callbacks.erase(remove_if(_callbacks.begin(), _callbacks.end(), MODBUS_COMPARE_ON), _callbacks.end());
    address++;
  }
#else
#define MODBUS_COMPARE_ON                                                                                                                            \
  [callbackType, address, callbackFunction](const TCallback entry) {                                                                                 \
    return entry.type == callbackType && entry.address == address && (!callbackFunction || entry.callback == callbackFunction);                      \
  }
  while (registerCount--) {
    size_t callbackIndex = 0;
    do {
      callbackIndex = _callbacks.find(MODBUS_COMPARE_ON);
      _callbacks.remove(callbackIndex);
    } while (callbackIndex < _callbacks.size());
    address++;
  }
#endif
  return originalSize == _callbacks.size();
}
bool Modbus::removeOnSet(TAddress address, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOn(TCallback::ON_SET, address, callbackFunction, registerCount);
}

bool Modbus::removeOnGet(TAddress address, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOn(TCallback::ON_GET, address, callbackFunction, registerCount);
}

bool Modbus::readSlave(uint16_t address, uint16_t registerCount, FunctionCode functionCode) {
  free(_frame);
  _len = 5;
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) {
    _reply = REPLY_OFF;
    return false;
  }
  _frame[0] = functionCode;
  _frame[1] = address >> 8;
  _frame[2] = address & 0x00FF;
  _frame[3] = registerCount >> 8;
  _frame[4] = registerCount & 0x00FF;
  return true;
}

bool Modbus::writeSlaveBits(TAddress startRegister, uint16_t destination, uint16_t registerCount, FunctionCode functionCode, bool* data) {
  free(_frame);
  _len = 6 + registerCount / 8;
  if (registerCount % 8) _len++; // Add 1 to the message length for the partial byte.
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) {
    _reply = REPLY_OFF;
    return false;
  }
  _frame[0] = functionCode;
  _frame[1] = destination >> 8;
  _frame[2] = destination & 0x00FF;
  _frame[3] = registerCount >> 8;
  _frame[4] = registerCount & 0x00FF;
  _frame[5] = _len - 6;
  _frame[_len - 1] = 0; // Clean last probably partial byte
  if (data) {
    boolToBits(_frame + 6, data, registerCount);
  } else {
    getMultipleBits(_frame + 6, startRegister, registerCount);
  }
  _reply = REPLY_NORMAL;
  return true;
}

bool Modbus::writeSlaveWords(TAddress startRegister, uint16_t destination, uint16_t registerCount, FunctionCode functionCode, uint16_t* data) {
  free(_frame);
  _len = 6 + 2 * registerCount;
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) {
    _reply = REPLY_OFF;
    return false;
  }
  _frame[0] = functionCode;
  _frame[1] = destination >> 8;
  _frame[2] = destination & 0x00FF;
  _frame[3] = registerCount >> 8;
  _frame[4] = registerCount & 0x00FF;
  _frame[5] = _len - 6;
  if (data) {
    uint16_t* frame = (uint16_t*)(_frame + 6);
    for (uint8_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
      frame[registerIndex] = __swap_16(data[registerIndex]);
    }
  } else {
    getMultipleWords((uint16_t*)(_frame + 6), startRegister, registerCount);
  }
  return true;
}

void Modbus::boolToBits(uint8_t* dst, bool* src, uint16_t registerCount) {
  uint8_t bitIndex = 0;
  uint16_t byteIndex = 0;
  uint16_t registerIndex = 0;
  while (registerCount--) {
    if (src[registerIndex]) bitSet(dst[byteIndex], bitIndex);
    else bitClear(dst[byteIndex], bitIndex);
    bitIndex++;
    if (bitIndex == 8) {
      byteIndex++;
      bitIndex = 0;
    }
    registerIndex++;
  }
}

void Modbus::bitsToBool(bool* dst, uint8_t* src, uint16_t registerCount) {
  uint8_t bitIndex = 0;
  uint16_t byteIndex = 0;
  uint16_t registerIndex = 0;
  while (registerCount--) {
    dst[registerIndex] = bitRead(src[byteIndex], bitIndex);
    bitIndex++;
    if (bitIndex == 8) {
      byteIndex++;
      bitIndex = 0;
    }
    registerIndex++;
  }
}

void Modbus::masterPDU(uint8_t* frame, uint8_t* sourceFrame, TAddress startRegister, uint8_t* output) {
  uint8_t fcode = frame[0];
  if ((fcode & 0x80) != 0) { // Check for an error response.
    _reply = frame[1];
    return;
  }
  if (fcode != sourceFrame[0]) { // Check whether the response matches the request.
    _reply = EX_DATA_MISMATCH;
    return;
  }
  _reply = EX_SUCCESS;
  uint16_t field2 = (uint16_t)sourceFrame[3] << 8 | (uint16_t)sourceFrame[4];
  uint8_t calculatedByteCount;
  switch (fcode) {
    case FC_READ_REGS:
    case FC_READ_INPUT_REGS:
    case FC_READWRITE_REGS:
      // field2 = registerCount, frame[1] = data length, header length = 2
      if (frame[1] != 2 * field2) { // Check if data size matches
        _reply = EX_DATA_MISMATCH;
        break;
      }
      if (output) {
        uint16_t* source = (uint16_t*)(frame + 2);
        uint16_t* destination = (uint16_t*)output;
        while (field2--) {
          *(destination++) = __swap_16(*(source++));
        }
      } else {
        setMultipleWords((uint16_t*)(frame + 2), startRegister, field2);
      }
      break;
    case FC_READ_COILS:
    case FC_READ_INPUT_STAT:
      // field2 = registerCount, frame[1] = data length, header length = 2
      calculatedByteCount = field2 / 8;
      if (field2 % 8) calculatedByteCount++;
      if (frame[1] != calculatedByteCount) { // Check whether the data size matches.
        _reply = EX_DATA_MISMATCH;
        break;
      }
      if (output) {
        bitsToBool((bool*)output, frame + 2, field2);
      } else {
        setMultipleBits(frame + 2, startRegister, field2);
      }
      break;
#if defined(MODBUS_FILES)
    case FC_READ_FILE_REC:
      // TODO: Determine whether the byte order must be swapped.
      if (frame[1] < 0x07 || frame[1] > 0xF5) { // Wrong request data size
        _reply = EX_ILLEGAL_VALUE;
        return;
      }
      {
        uint8_t* data = frame + 2;
        uint8_t* eoFrame = frame + frame[1];
        while (data < eoFrame) {
          // data[0] = subrequest length
          // data[1] = 0x06
          if (data[1] != 0x06 || data[0] < 0x07 || data[0] > 0xF5 || data + data[0] > eoFrame) { // Wrong request data size
            _reply = EX_ILLEGAL_VALUE;
            return;
          }
          memcpy(output, data + 2, data[0]);
          data += data[0] + 1;
          output += data[0] - 1;
        }
      }
      break;
    case FC_WRITE_FILE_REC:
#endif
    case FC_WRITE_REG:
    case FC_WRITE_REGS:
    case FC_WRITE_COIL:
    case FC_WRITE_COILS:
    case FC_MASKWRITE_REG: break;

    default: _reply = EX_GENERAL_FAILURE;
  }
}

void Modbus::cbEnable(bool state) { cbEnabled = state; }
void Modbus::cbDisable() { cbEnable(false); }
Modbus::~Modbus() { free(_frame); }

#if defined(MODBUS_FILES)
#if defined(MODBUS_USE_STL)
bool Modbus::onFile(std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> callbackFunction) {
#else
bool Modbus::onFile(Modbus::ResultCode (*callbackFunction)(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)) {
#endif
  _onFile = callbackFunction;
  return true;
}
Modbus::ResultCode Modbus::fileOp(Modbus::FunctionCode functionCode, uint16_t fileNum, uint16_t recNum, uint16_t recLen, uint8_t* frame) {
  if (!_onFile) return EX_ILLEGAL_ADDRESS;
  return _onFile(functionCode, fileNum, recNum, recLen, frame);
}

bool Modbus::readSlaveFile(uint16_t* fileNum, uint16_t* startRec, uint16_t* len, uint8_t count, FunctionCode functionCode) {
  _len = count * 7 + 2;
  if (_len > MODBUS_MAX_FRAME) return false;
  free(_frame);
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) return false;
  _frame[0] = functionCode;
  _frame[1] = _len - 2;
  uint8_t* subReq = _frame + 2;
  for (uint8_t recordIndex = 0; recordIndex < count; recordIndex++) {
    subReq[0] = 0x06;
    subReq[1] = fileNum[recordIndex] >> 8;
    subReq[2] = fileNum[recordIndex] & 0x00FF;
    subReq[3] = startRec[recordIndex] >> 8;
    subReq[4] = startRec[recordIndex] & 0x00FF;
    subReq[5] = len[recordIndex] >> 8;
    subReq[6] = len[recordIndex] & 0x00FF;
    subReq += 7;
  }
  return true;
}
bool Modbus::writeSlaveFile(uint16_t* fileNum, uint16_t* startRec, uint16_t* len, uint8_t count, FunctionCode functionCode, uint8_t* data) {
  _len = 2;
  for (uint8_t recordIndex = 0; recordIndex < count; recordIndex++) {
    _len += len[recordIndex] * 2 + 7;
  }
  if (_len > MODBUS_MAX_FRAME) return false;
  free(_frame);
  _frame = (uint8_t*)malloc(_len);
  if (!_frame) return false;
  _frame[0] = functionCode;
  _frame[1] = _len - 2;
  uint8_t* subReq = _frame + 2;
  for (uint8_t recordIndex = 0; recordIndex < count; recordIndex++) {
    subReq[0] = 0x06;
    subReq[1] = fileNum[recordIndex] >> 8;
    subReq[2] = fileNum[recordIndex] & 0x00FF;
    subReq[3] = startRec[recordIndex] >> 8;
    subReq[4] = startRec[recordIndex] & 0x00FF;
    subReq[5] = len[recordIndex] >> 8;
    subReq[6] = len[recordIndex] & 0x00FF;
    uint8_t contentLength = len[recordIndex] * 2;
    memcpy(subReq + 7, data, contentLength);
    subReq += 7 + contentLength;
    data += contentLength;
  }
  return true;
}
#endif

bool Modbus::onRaw(cbRaw callbackFunction) {
  _cbRaw = callbackFunction;
  return true;
}
Modbus::ResultCode Modbus::_onRequestDefault(Modbus::FunctionCode functionCode, const RequestData data) {
  (void)functionCode;
  (void)data;
  return EX_SUCCESS;
}
bool Modbus::onRequest(cbRequest callbackFunction) {
  _onRequest = callbackFunction;
  return true;
}
#if defined(MODBUSAPI_OPTIONAL)
bool Modbus::onRequestSuccess(cbRequest callbackFunction) {
  _onRequestSuccess = callbackFunction;
  return true;
}
#endif

#if defined(ARDUINO_SAM_DUE_STL)
namespace std {
void __throw_bad_function_call() {
  Serial.println(F("STL ERROR - __throw_bad_function_call"));
  __builtin_unreachable();
}
} // namespace std
#endif
