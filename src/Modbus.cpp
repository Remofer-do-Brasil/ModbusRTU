/*
    Modbus RTU library
    Core functions
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2021 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "Modbus.h"
#include <algorithm>
#include <cstring>

uint16_t Modbus::callback(TRegister* reg, uint16_t value, TCallback::CallbackType callbackType) {
  for (const auto& entry : _callbacks) {
    if (entry.address == reg->address && entry.type == callbackType) {
      value = entry.callback(reg, value);
    }
  }
  return value;
}

auto Modbus::registerPosition(TAddress address) -> std::vector<TRegister>::iterator {
  const auto addressLess = [](TAddress lhs, TAddress rhs) {
    if (lhs.type != rhs.type) {
      return lhs.type < rhs.type;
    }
    return lhs.address < rhs.address;
  };
  return std::ranges::lower_bound(_regs, address, addressLess, &TRegister::address);
}

TRegister* Modbus::searchRegister(TAddress address) {
  auto const pos = registerPosition(address);
  return pos != _regs.end() && pos->address == address ? &*pos : nullptr;
}

bool Modbus::addReg(TAddress address, uint16_t value, uint16_t registerCount) {
  if (!registerCount || static_cast<uint32_t>(address.address) + registerCount > 65536) {
    return false;
  }
  auto pos = registerPosition(address);
  while (registerCount--) {
    if (pos == _regs.end() || pos->address != address) {
      pos = _regs.insert(pos, { .address = address, .value = value });
    }
    ++pos;
    ++address;
  }
  return true;
}

bool Modbus::Reg(TAddress address, uint16_t value) {
  TRegister* reg = nullptr;
  reg = searchRegister(address); // Search for the register address.
  if (reg) {                     // If found, assign the new register value.
    if (cbEnabled) {
      reg->value = callback(reg, value, TCallback::ON_SET);
    } else {
      reg->value = value;
    }
    return true;
  }
  return false;
}

uint16_t Modbus::Reg(TAddress address) {
  TRegister* reg = nullptr;
  reg = searchRegister(address);
  if (reg) {
    if (cbEnabled) {
      return callback(reg, reg->value, TCallback::ON_GET);
    } 
      return reg->value;
    
  } 
    return 0;
  
}

bool Modbus::removeReg(TAddress address, uint16_t registerCount) {
  bool atLeastOne = false;
  if (!registerCount || static_cast<uint32_t>(address.address) + registerCount > 65536) {
    return false;
  }
  for (uint16_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
    auto const pos = registerPosition(address + registerIndex);
    if (pos != _regs.end() && pos->address == address + registerIndex) {
      atLeastOne = true;
      removeOnSet(address + registerIndex);
      removeOnGet(address + registerIndex);
      _regs.erase(pos);
    }
  }
  return atLeastOne;
}

bool Modbus::addReg(TAddress address, uint16_t* value, uint16_t registerCount) {
  if (!value || !registerCount || static_cast<uint32_t>(address.address) + registerCount > 65536) {
    return false;
  }
  for (uint16_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
    addReg(address + registerIndex, value[registerIndex]);
  }
  return true;
}

bool Modbus::Reg(TAddress address, uint16_t* values, uint16_t registerCount) {
  if (!values || static_cast<uint32_t>(address.address) + registerCount > 65536) {
    return false;
  }
  for (uint16_t i = 0; i < registerCount; ++i) {
    if (!Reg(address + i, values[i])) {
      return false;
    }
  }
  return true;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void Modbus::slavePDU(uint8_t* frame) {
  auto const fcode = static_cast<FunctionCode>(frame[0]);
  uint16_t const field1 = _len >= 3 ? modbus_detail::readWord(frame + 1) : 0;
  uint16_t const field2 = _len >= 5 ? modbus_detail::readWord(frame + 3) : 0;
  uint16_t field3 = 0;
  uint16_t field4 = 0;
  uint16_t calculatedByteCount = 0;
  uint16_t registerIndex = 0;
  ResultCode resultCode{};
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
      if (field2 < 0x0001 || field2 > MODBUS_WRITE_WORDS || 65536 - field1 < field2 || frame[5] != 2 * field2) { // Check constraints.
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      for (registerIndex = 0; registerIndex < field2; registerIndex++) { // Check Address (startRegister...startRegister + registerCount)
        if (!searchRegister(HREG(field1) + registerIndex)) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
      }
      if (!setMultipleWords(frame + 6, HREG(field1), field2)) {
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
      if (field2 % 8) {
        calculatedByteCount++;
      }
      if (field2 < 0x0001 || field2 > MODBUS_WRITE_BITS || 65536 - field1 < field2 ||
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
    case FC_READ_FILE_REC:
      if (frame[1] < 0x07 || frame[1] > 0xF5) { // Wrong request data size
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      {
        unsigned bufSize = 2;                                                   // 2 bytes for frame header
        uint8_t const* recs = frame + 2;                                        // Begin of sub-recs blocks
        uint8_t const recsCount = frame[1] / 7;                                 // Number of sub request blocks.
        for (uint8_t recordIndex = 0; recordIndex < recsCount; recordIndex++) { // Calculate the required output-buffer size.
          // uint16_t fileNum = (uint16_t)recs[1] << 8 | (uint16_t)recs[2];
          uint16_t const recNum = static_cast<uint16_t>(static_cast<uint16_t>(recs[3]) << 8U) | static_cast<uint16_t>(recs[4]);
          uint16_t const recLen = static_cast<uint16_t>(static_cast<uint16_t>(recs[5]) << 8U) | static_cast<uint16_t>(recs[6]);
          if (recs[0] != 0x06 || !recLen || recNum > 0x270F) { // Invalid reference type or record count.
            exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
            return;
          }
          bufSize += (recLen * 2) + 2; // 4 bytes for header + data
          recs += 7;
        }
        if (bufSize > MODBUS_MAX_FRAME) { // Frame to return too large
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }

        _len = bufSize;
        recs = frame + 2; // Begin of sub-recs blocks
        uint8_t* data = _frame + 2;
        for (uint8_t recordIndex = 0; recordIndex < recsCount; recordIndex++) {
          uint16_t const fileNum = static_cast<uint16_t>(static_cast<uint16_t>(recs[1]) << 8U) | static_cast<uint16_t>(recs[2]);
          uint16_t const recNum = static_cast<uint16_t>(static_cast<uint16_t>(recs[3]) << 8U) | static_cast<uint16_t>(recs[4]);
          uint16_t const recLen = static_cast<uint16_t>(static_cast<uint16_t>(recs[5]) << 8U) | static_cast<uint16_t>(recs[6]);
          ResultCode const res = fileOp(fcode, fileNum, recNum, recLen, data + 2);
          if (res != EX_SUCCESS) { // File read failed
            exceptionResponse(fcode, res);
            return;
          }
          data[0] = (recLen * 2) + 1;
          data[1] = 0x06;
          data += (recLen * 2) + 2;
          recs += 7;
        }
        _frame[0] = fcode;
        _frame[1] = bufSize - 2;
        _reply = REPLY_NORMAL;
      }
      break;
    case FC_WRITE_FILE_REC: {
      if (frame[1] < 0x09 || frame[1] > 0xFB) { // Wrong request data size
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      uint8_t* recs = frame + 2; // Begin of sub-recs blocks
      while (recs < frame + _len) {
        if (frame + _len - recs < 7) {
          exceptionResponse(fcode, EX_ILLEGAL_VALUE);
          return;
        }
        if (recs[0] != 0x06) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
        uint16_t const fileNum = static_cast<uint16_t>(static_cast<uint16_t>(recs[1]) << 8U) | static_cast<uint16_t>(recs[2]);
        uint16_t const recNum = static_cast<uint16_t>(static_cast<uint16_t>(recs[3]) << 8U) | static_cast<uint16_t>(recs[4]);
        uint16_t const recLen = static_cast<uint16_t>(static_cast<uint16_t>(recs[5]) << 8U) | static_cast<uint16_t>(recs[6]);
        if (recLen == 0 || 2 * recLen > frame + _len - recs - 7) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
        ResultCode const res = fileOp(fcode, fileNum, recNum, recLen, recs + 7);
        if (res != EX_SUCCESS) { // File write failed
          exceptionResponse(fcode, res);
          return;
        }
        recs += 7 + (recLen * 2);
      }
    }
      _reply = REPLY_ECHO;
      break;
    case FC_MASKWRITE_REG:
      // field1 = reg, field2 = AND mask, field3 = OR mask
      //  Result = (Current Contents AND And_Mask) OR (Or_Mask AND (NOT And_Mask))
      field3 = static_cast<uint16_t>(static_cast<uint16_t>(frame[5]) << 8U) | static_cast<uint16_t>(frame[6]);
      resultCode = _onRequest(fcode, { HREG(field1), field2, field3 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      field4 = Reg(HREG(field1));
      field4 = static_cast<uint16_t>(field4 & field2) | static_cast<uint16_t>(field3 & static_cast<uint16_t>(~field2));
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
      field3 = static_cast<uint16_t>(static_cast<uint16_t>(frame[5]) << 8U) | static_cast<uint16_t>(frame[6]);
      field4 = static_cast<uint16_t>(static_cast<uint16_t>(frame[7]) << 8U) | static_cast<uint16_t>(frame[8]);
      resultCode = _onRequest(fcode, { HREG(field1), field2, HREG(field3), field4 });
      if (resultCode != EX_SUCCESS) {
        exceptionResponse(fcode, resultCode);
        return;
      }
      if (field2 < 0x0001 || field2 > MODBUS_MAX_WORDS || field4 < 0x0001 || field4 > 121 || 65536 - field1 < field2 || 65536 - field3 < field4 ||
          frame[9] != 2 * field4) { // Check value
        exceptionResponse(fcode, EX_ILLEGAL_VALUE);
        return;
      }
      for (uint16_t i = 0; i < field2; ++i) {
        if (!searchRegister(HREG(field1) + i)) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
      }
      for (uint16_t i = 0; i < field4; ++i) {
        if (!searchRegister(HREG(field3) + i)) {
          exceptionResponse(fcode, EX_ILLEGAL_ADDRESS);
          return;
        }
      }
      if (!setMultipleWords(frame + 10, HREG(field3), field4)) {
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
  _len = 5;
  _frame[0] = functionCode;
  _frame[1] = startRegister.address >> 8U;
  _frame[2] = startRegister.address & 0x00FFU;
  _frame[3] = outputCount >> 8U;
  _frame[4] = outputCount & 0x00FFU;
}

void Modbus::exceptionResponse(FunctionCode functionCode, ResultCode exceptionCode) {
  _len = 2;
  _frame[0] = functionCode + 0x80;
  _frame[1] = exceptionCode;
  _reply = REPLY_NORMAL;
}

void Modbus::getMultipleBits(uint8_t* frame, TAddress startRegister, uint16_t registerCount) {
  uint8_t bitIndex = 0;
  uint16_t byteIndex = 0;
  while (registerCount--) {
    if (BIT_BOOL(Reg(startRegister))) {
      frame[byteIndex] |= 1U << bitIndex;
    } else {
      frame[byteIndex] &= ~(1U << bitIndex);
    }
    bitIndex++;
    if (bitIndex == 8) {
      byteIndex++;
      bitIndex = 0;
    }
    startRegister++; // increment the register
  }
}

void Modbus::getMultipleWords(uint8_t* frame, TAddress startRegister, uint16_t registerCount) {
  for (uint16_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
    modbus_detail::writeWord(frame + (2 * registerIndex), Reg(startRegister + registerIndex));
  }
}

Modbus::ResultCode Modbus::readBits(TAddress startRegister, uint16_t registerCount, FunctionCode functionCode) {
  if (registerCount < 0x0001 || registerCount > MODBUS_MAX_BITS || (65536 - startRegister.address) < registerCount) {
    return EX_ILLEGAL_VALUE;
  }
  for (uint16_t registerIndex = 0; registerIndex < registerCount;
       registerIndex++) { // Check addresses from startRegister through startRegister + registerCount.
    if (!searchRegister(startRegister + registerIndex)) {
      return EX_ILLEGAL_ADDRESS;
    }
  }
  // Determine the message length = function type, byte count and
  // for each group of 8 registers the message length increases by 1
  _len = 2 + (registerCount / 8);
  if (registerCount % 8) {
    _len++; // Add 1 to the message length for the partial byte.
  }
  _frame[0] = functionCode;
  _frame[1] = _len - 2; // byte count (_len - function code and byte count)
  _frame[_len - 1] = 0; // Clean last probably partial byte
  getMultipleBits(_frame + 2, startRegister, registerCount);
  _reply = REPLY_NORMAL;
  return EX_SUCCESS;
}

Modbus::ResultCode Modbus::readWords(TAddress startRegister, uint16_t registerCount, FunctionCode functionCode) {
  // Check value (registerCount)
  if (registerCount < 0x0001 || registerCount > MODBUS_MAX_WORDS || 65536 - startRegister.address < registerCount) {
    return EX_ILLEGAL_VALUE;
  }
  for (uint16_t registerIndex = 0; registerIndex < registerCount;
       registerIndex++) { // Check addresses from startRegister through startRegister + registerCount.
    if (!searchRegister(startRegister + registerIndex)) {
      return EX_ILLEGAL_ADDRESS;
    }
  }
  _len = 2 + (registerCount * 2); // Two bytes per register plus two header bytes.
  _frame[0] = functionCode;
  _frame[1] = _len - 2; // byte count
  getMultipleWords(_frame + 2, startRegister, registerCount);
  _reply = REPLY_NORMAL;
  return EX_SUCCESS;
}

bool Modbus::setMultipleBits(const uint8_t* frame, TAddress startRegister, uint16_t outputCount) {
  uint8_t bitIndex = 0;
  uint16_t byteIndex = 0;
  bool result = true;
  while (outputCount--) {
    Reg(startRegister, BIT_VAL((frame[byteIndex] & (1U << bitIndex))));
    if (Reg(startRegister) != BIT_VAL((frame[byteIndex] & (1U << bitIndex)))) {
      result = false;
    }
    bitIndex++;
    if (bitIndex == 8) {
      byteIndex++;
      bitIndex = 0;
    }
    startRegister++; // increment the register
  }
  return result;
}

bool Modbus::setMultipleWords(const uint8_t* frame, TAddress startRegister, uint16_t registerCount) {
  bool result = true;
  for (uint16_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
    Reg(startRegister + registerIndex, modbus_detail::readWord(frame + (2 * registerIndex)));
    if (Reg(startRegister + registerIndex) != modbus_detail::readWord(frame + (2 * registerIndex))) {
      result = false;
    }
  }
  return result;
}

bool Modbus::addOn(TCallback::CallbackType callbackType, TAddress address, const cbModbus& callbackFunction, uint16_t registerCount, CallbackId id) {
  const auto originalSize = _callbacks.size();
  while (registerCount--) {
    if (searchRegister(address)) {
      _callbacks.push_back({ .type = callbackType, .address = address, .callback = callbackFunction, .id = id });
    }
    ++address;
  }
  return _callbacks.size() != originalSize;
}

bool Modbus::onGet(TAddress address, const cbModbus& callbackFunction, uint16_t registerCount) {
  return callbackFunction ? addOn(TCallback::ON_GET, address, callbackFunction, registerCount) : removeOnGet(address);
}

bool Modbus::onSet(TAddress address, const cbModbus& callbackFunction, uint16_t registerCount) {
  return callbackFunction ? addOn(TCallback::ON_SET, address, callbackFunction, registerCount) : removeOnSet(address);
}

std::optional<Modbus::CallbackId>
Modbus::addOnWithId(TCallback::CallbackType callbackType, TAddress address, const cbModbus& callbackFunction, uint16_t registerCount) {
  if (!callbackFunction || !registerCount || static_cast<uint32_t>(address.address) + registerCount > 65536 || _nextCallbackId == 0) {
    return std::nullopt;
  }

  // Zero is reserved. Once the counter wraps, stop issuing IDs to avoid reusing a handle.
  const CallbackId id{ .value = _nextCallbackId++ };
  // A throwing callback copy can leave a partially registered range on exception-enabled builds.
  struct RegistrationRollback {
    Modbus* modbus = nullptr;
    CallbackId id{};
    bool committed = false;

    ~RegistrationRollback() {
      if (!committed) {
        modbus->removeCallback(id);
      }
    }
  } rollback{ .modbus = this, .id = id };

  const bool added = addOn(callbackType, address, callbackFunction, registerCount, id);
  rollback.committed = true;
  return added ? std::optional{ id } : std::nullopt;
}

std::optional<Modbus::CallbackId> Modbus::onGetWithId(TAddress address, const cbModbus& callbackFunction, uint16_t registerCount) {
  return addOnWithId(TCallback::ON_GET, address, callbackFunction, registerCount);
}

std::optional<Modbus::CallbackId> Modbus::onSetWithId(TAddress address, const cbModbus& callbackFunction, uint16_t registerCount) {
  return addOnWithId(TCallback::ON_SET, address, callbackFunction, registerCount);
}

bool Modbus::removeCallback(CallbackId id) {
  if (id.value == 0) {
    return false;
  }
  const auto end = _callbacks.end();
  const auto firstRemoved = std::remove_if(_callbacks.begin(), end, [id](const TCallback& entry) { return entry.id == id; });
  const bool removed = firstRemoved != end;
  _callbacks.erase(firstRemoved, end);
  return removed;
}

bool Modbus::removeOn(TCallback::CallbackType callbackType, TAddress address, const cbModbus& callbackFunction, uint16_t registerCount) {
  if (callbackFunction) {
    // Preserve the legacy return value when no callback matches.
    return true;
  }
  const auto originalSize = _callbacks.size();
  while (registerCount--) {
    const auto end = _callbacks.end();
    const auto firstRemoved = std::remove_if(
        _callbacks.begin(), end, [callbackType, address](const TCallback& entry) { return entry.type == callbackType && entry.address == address; });
    _callbacks.erase(firstRemoved, end);
    ++address;
  }
  return originalSize == _callbacks.size();
}
bool Modbus::removeOnSet(TAddress address, const cbModbus& callbackFunction, uint16_t registerCount) {
  return removeOn(TCallback::ON_SET, address, callbackFunction, registerCount);
}

bool Modbus::removeOnGet(TAddress address, const cbModbus& callbackFunction, uint16_t registerCount) {
  return removeOn(TCallback::ON_GET, address, callbackFunction, registerCount);
}

bool Modbus::readSlave(uint16_t address, uint16_t registerCount, FunctionCode functionCode) {
  _len = 5;
  _frame[0] = functionCode;
  _frame[1] = address >> 8U;
  _frame[2] = address & 0x00FFU;
  _frame[3] = registerCount >> 8U;
  _frame[4] = registerCount & 0x00FFU;
  return true;
}

bool Modbus::writeSlaveBits(TAddress startRegister, uint16_t destination, uint16_t registerCount, FunctionCode functionCode, bool* data) {
  _len = 6 + (registerCount / 8);
  if (registerCount % 8) {
    _len++; // Add 1 to the message length for the partial byte.
  }
  _frame[0] = functionCode;
  _frame[1] = destination >> 8U;
  _frame[2] = destination & 0x00FFU;
  _frame[3] = registerCount >> 8U;
  _frame[4] = registerCount & 0x00FFU;
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
  _len = 6 + (2 * registerCount);
  _frame[0] = functionCode;
  _frame[1] = destination >> 8U;
  _frame[2] = destination & 0x00FFU;
  _frame[3] = registerCount >> 8U;
  _frame[4] = registerCount & 0x00FFU;
  _frame[5] = _len - 6;
  if (data) {
    uint8_t* frame = _frame + 6;
    for (uint16_t registerIndex = 0; registerIndex < registerCount; registerIndex++) {
      modbus_detail::writeWord(frame + (2 * registerIndex), data[registerIndex]);
    }
  } else {
    getMultipleWords(_frame + 6, startRegister, registerCount);
  }
  return true;
}

void Modbus::boolToBits(uint8_t* dst, const bool* src, uint16_t registerCount) {
  uint8_t bitIndex = 0;
  uint16_t byteIndex = 0;
  uint16_t registerIndex = 0;
  while (registerCount--) {
    if (src[registerIndex]) {
      dst[byteIndex] |= 1U << bitIndex;
    } else {
      dst[byteIndex] &= ~(1U << bitIndex);
    }
    bitIndex++;
    if (bitIndex == 8) {
      byteIndex++;
      bitIndex = 0;
    }
    registerIndex++;
  }
}

void Modbus::bitsToBool(bool* dst, const uint8_t* src, uint16_t registerCount) {
  uint8_t bitIndex = 0;
  uint16_t byteIndex = 0;
  uint16_t registerIndex = 0;
  while (registerCount--) {
    dst[registerIndex] = (src[byteIndex] & (1U << bitIndex));
    bitIndex++;
    if (bitIndex == 8) {
      byteIndex++;
      bitIndex = 0;
    }
    registerIndex++;
  }
}

void Modbus::masterPDU(uint8_t* frame, const uint8_t* sourceFrame, TAddress startRegister, uint8_t* output) {
  uint8_t const fcode = frame[0];
  if ((fcode & 0x80U) != 0) { // Check for an error response.
    _reply = frame[1];
    return;
  }
  if (fcode != sourceFrame[0]) { // Check whether the response matches the request.
    _reply = EX_DATA_MISMATCH;
    return;
  }
  _reply = EX_SUCCESS;
  uint16_t field2 = static_cast<uint16_t>(sourceFrame[3] << 8U) | static_cast<uint16_t>(sourceFrame[4]);
  unsigned calculatedByteCount = 0;
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
        const uint8_t* source = frame + 2;
        auto* destination = reinterpret_cast<uint16_t*>(output);
        while (field2--) {
          *destination = modbus_detail::readWord(source);
          ++destination;
          source += 2;
        }
      } else {
        setMultipleWords(frame + 2, startRegister, field2);
      }
      break;
    case FC_READ_COILS:
    case FC_READ_INPUT_STAT:
      // field2 = registerCount, frame[1] = data length, header length = 2
      calculatedByteCount = field2 / 8;
      if (field2 % 8) {
        calculatedByteCount++;
      }
      if (frame[1] != calculatedByteCount) { // Check whether the data size matches.
        _reply = EX_DATA_MISMATCH;
        break;
      }
      if (output) {
        bitsToBool(reinterpret_cast<bool*>(output), frame + 2, field2);
      } else {
        setMultipleBits(frame + 2, startRegister, field2);
      }
      break;
    case FC_READ_FILE_REC:
      // The client API requests one file record per transaction.
      if (!output || _len < 4 || frame[1] != _len - 2 || frame[2] != _len - 3 || frame[3] != 6 ||
          // NOLINTNEXTLINE(bugprone-signed-bitwise)
          _len - 4 != 2 * ((sourceFrame[7] << 8) | sourceFrame[8])) {
        _reply = EX_DATA_MISMATCH;
      } else {
        memcpy(output, frame + 4, _len - 4);
      }
      break;
    case FC_WRITE_FILE_REC:
    case FC_WRITE_REG:
    case FC_WRITE_REGS:
    case FC_WRITE_COIL:
    case FC_WRITE_COILS:
    case FC_MASKWRITE_REG:  break;

    default: _reply = EX_GENERAL_FAILURE;
  }
}

void Modbus::cbEnable(bool state) { cbEnabled = state; }
void Modbus::cbDisable() { cbEnable(false); }

bool Modbus::onFile(const std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)>& callbackFunction) {
  _onFile = callbackFunction;
  return true;
}
Modbus::ResultCode Modbus::fileOp(Modbus::FunctionCode functionCode, uint16_t fileNum, uint16_t recNum, uint16_t recLen, uint8_t* frame) {
  if (!_onFile) {
    return EX_ILLEGAL_ADDRESS;
  }
  return _onFile(functionCode, fileNum, recNum, recLen, frame);
}

bool Modbus::readSlaveFile(const uint16_t* fileNum, const uint16_t* startRec, const uint16_t* len, uint8_t count, FunctionCode functionCode) {
  if (count != 1 || len[0] > 124) {
    return false;
  }
  _len = (count * 7) + 2;
  if (_len > MODBUS_MAX_FRAME) {
    return false;
  }
  _frame[0] = functionCode;
  _frame[1] = _len - 2;
  uint8_t* subReq = _frame + 2;
  for (uint8_t recordIndex = 0; recordIndex < count; recordIndex++) {
    subReq[0] = 0x06U;
    subReq[1] = fileNum[recordIndex] >> 8U;
    subReq[2] = fileNum[recordIndex] & 0x00FFU;
    subReq[3] = startRec[recordIndex] >> 8U;
    subReq[4] = startRec[recordIndex] & 0x00FFU;
    subReq[5] = len[recordIndex] >> 8U;
    subReq[6] = len[recordIndex] & 0x00FFU;
    subReq += 7;
  }
  return true;
}
bool Modbus::writeSlaveFile(const uint16_t* fileNum, const uint16_t* startRec, const uint16_t* len, uint8_t count, FunctionCode functionCode,
                            uint8_t* data) {
  if (count != 1 || len[0] > 122) {
    return false;
  }
  _len = 2;
  for (uint8_t recordIndex = 0; recordIndex < count; recordIndex++) {
    _len += (len[recordIndex] * 2) + 7;
  }
  if (_len > MODBUS_MAX_FRAME) {
    return false;
  }
  _frame[0] = functionCode;
  _frame[1] = _len - 2;
  uint8_t* subReq = _frame + 2;
  for (uint8_t recordIndex = 0; recordIndex < count; recordIndex++) {
    subReq[0] = 0x06U;
    subReq[1] = fileNum[recordIndex] >> 8U;
    subReq[2] = fileNum[recordIndex] & 0x00FFU;
    subReq[3] = startRec[recordIndex] >> 8U;
    subReq[4] = startRec[recordIndex] & 0x00FFU;
    subReq[5] = len[recordIndex] >> 8U;
    subReq[6] = len[recordIndex] & 0x00FFU;
    uint8_t const contentLength = len[recordIndex] * 2;
    memcpy(subReq + 7, data, contentLength);
    subReq += 7 + contentLength;
    data += contentLength;
  }
  return true;
}

bool Modbus::onRaw(const cbRaw& callbackFunction) {
  _cbRaw = callbackFunction;
  return true;
}
Modbus::ResultCode Modbus::_onRequestDefault(Modbus::FunctionCode functionCode, const RequestData data) {
  (void)functionCode;
  (void)data;
  return EX_SUCCESS;
}
bool Modbus::onRequest(const cbRequest& callbackFunction) {
  _onRequest = callbackFunction;
  return true;
}
bool Modbus::onRequestSuccess(const cbRequest& callbackFunction) {
  _onRequestSuccess = callbackFunction;
  return true;
}
