/*
    Modbus RTU library
        Modbus public API implementation
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2021 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#include "ModbusRTU_PICO.h"

#define IMPLEMENT_WRITEREG(FNAME, REG, FUNC, VALUE, VALTYPE)                                                                                         \
  uint16_t ModbusRTU::FNAME(uint8_t target, uint16_t offset, VALTYPE value, cbTransaction callbackFunction) {                                        \
    readSlave(offset, VALUE(value), Modbus::FUNC);                                                                                                   \
    return send(target, REG(offset), callbackFunction);                                                                                              \
  }
IMPLEMENT_WRITEREG(writeCoil, COIL, FC_WRITE_COIL, COIL_VAL, bool)
IMPLEMENT_WRITEREG(writeHreg, HREG, FC_WRITE_REG, , uint16_t)

#define IMPLEMENT_WRITEREGS(FNAME, REG, FUNC, VALUE, MAXNUM, VALTYPE)                                                                                \
  uint16_t ModbusRTU::FNAME(uint8_t target, uint16_t offset, VALTYPE* value, uint16_t registerCount, cbTransaction callbackFunction) {               \
    if (!registerCount || registerCount > MAXNUM || uint32_t(offset) + registerCount > 65536 || !value) return 0;                                    \
    VALUE(REG(offset), offset, registerCount, Modbus::FUNC, value);                                                                                  \
    return send(target, REG(offset), callbackFunction);                                                                                              \
  }
IMPLEMENT_WRITEREGS(writeCoil, COIL, FC_WRITE_COILS, writeSlaveBits, MODBUS_WRITE_BITS, bool)
IMPLEMENT_WRITEREGS(writeHreg, HREG, FC_WRITE_REGS, writeSlaveWords, MODBUS_WRITE_WORDS, uint16_t)

#define IMPLEMENT_READREGS(FNAME, REG, FUNC, MAXNUM, VALTYPE)                                                                                        \
  uint16_t ModbusRTU::FNAME(uint8_t target, uint16_t offset, VALTYPE* value, uint16_t registerCount, cbTransaction callbackFunction) {               \
    if (!registerCount || registerCount > MAXNUM || uint32_t(offset) + registerCount > 65536 || !value) return 0;                                    \
    readSlave(offset, registerCount, Modbus::FUNC);                                                                                                  \
    return send(target, REG(offset), callbackFunction, (uint8_t*)value);                                                                             \
  }
IMPLEMENT_READREGS(readCoil, COIL, FC_READ_COILS, 0x07D0, bool)
IMPLEMENT_READREGS(readHreg, HREG, FC_READ_REGS, 0x007D, uint16_t)
IMPLEMENT_READREGS(readIsts, ISTS, FC_READ_INPUT_STAT, 0x07D0, bool)
IMPLEMENT_READREGS(readIreg, IREG, FC_READ_INPUT_REGS, 0x007D, uint16_t)

#define IMPLEMENT_PULL(FNAME, REG, FUNC, MAXNUM)                                                                                                     \
  uint16_t ModbusRTU::FNAME(uint8_t target, uint16_t from, uint16_t destination, uint16_t registerCount, cbTransaction callbackFunction) {           \
    if (!registerCount || registerCount > MAXNUM || uint32_t(from) + registerCount > 65536 || uint32_t(destination) + registerCount > 65536)         \
      return 0;                                                                                                                                      \
    if (!addReg(REG(destination), uint16_t(0), registerCount)) return 0;                                                                             \
    readSlave(from, registerCount, Modbus::FUNC);                                                                                                    \
    return send(target, REG(destination), callbackFunction);                                                                                         \
  }
IMPLEMENT_PULL(pullCoil, COIL, FC_READ_COILS, 0x07D0)
IMPLEMENT_PULL(pullIsts, ISTS, FC_READ_INPUT_STAT, 0x07D0)
IMPLEMENT_PULL(pullHreg, HREG, FC_READ_REGS, 0x007D)
IMPLEMENT_PULL(pullIreg, IREG, FC_READ_INPUT_REGS, 0x007D)
IMPLEMENT_PULL(pullHregToIreg, IREG, FC_READ_REGS, 0x007D)
IMPLEMENT_PULL(pullCoilToIsts, ISTS, FC_READ_COILS, 0x07D0)

#define IMPLEMENT_PUSH(FNAME, REG, FUNC, MAXNUM, FINT)                                                                                               \
  uint16_t ModbusRTU::FNAME(uint8_t target, uint16_t destination, uint16_t from, uint16_t registerCount, cbTransaction callbackFunction) {           \
    if (!registerCount || registerCount > MAXNUM || uint32_t(from) + registerCount > 65536 || uint32_t(destination) + registerCount > 65536)         \
      return 0;                                                                                                                                      \
    if (!searchRegister(REG(from))) return false;                                                                                                    \
    FINT(REG(from), destination, registerCount, Modbus::FUNC);                                                                                       \
    return send(target, REG(from), callbackFunction);                                                                                                \
  }
IMPLEMENT_PUSH(pushCoil, COIL, FC_WRITE_COILS, MODBUS_WRITE_BITS, writeSlaveBits)
IMPLEMENT_PUSH(pushHreg, HREG, FC_WRITE_REGS, MODBUS_WRITE_WORDS, writeSlaveWords)
IMPLEMENT_PUSH(pushIregToHreg, IREG, FC_WRITE_REGS, MODBUS_WRITE_WORDS, writeSlaveWords)
IMPLEMENT_PUSH(pushIstsToCoil, ISTS, FC_WRITE_COILS, MODBUS_WRITE_BITS, writeSlaveBits)

bool ModbusRTU::addHreg(uint16_t offset, uint16_t value, uint16_t registerCount) { return addReg(HREG(offset), value, registerCount); }
bool ModbusRTU::Hreg(uint16_t offset, uint16_t value) { return Reg(HREG(offset), value); }
uint16_t ModbusRTU::Hreg(uint16_t offset) { return Reg(HREG(offset)); }
bool ModbusRTU::removeHreg(uint16_t offset, uint16_t registerCount) { return removeReg(HREG(offset), registerCount); }
bool ModbusRTU::addCoil(uint16_t offset, bool value, uint16_t registerCount) { return addReg(COIL(offset), COIL_VAL(value), registerCount); }
bool ModbusRTU::addIsts(uint16_t offset, bool value, uint16_t registerCount) { return addReg(ISTS(offset), ISTS_VAL(value), registerCount); }
bool ModbusRTU::addIreg(uint16_t offset, uint16_t value, uint16_t registerCount) { return addReg(IREG(offset), value, registerCount); }
bool ModbusRTU::Coil(uint16_t offset, bool value) { return Reg(COIL(offset), COIL_VAL(value)); }
bool ModbusRTU::Ists(uint16_t offset, bool value) { return Reg(ISTS(offset), ISTS_VAL(value)); }
bool ModbusRTU::Ireg(uint16_t offset, uint16_t value) { return Reg(IREG(offset), value); }
bool ModbusRTU::Coil(uint16_t offset) { return COIL_BOOL(Reg(COIL(offset))); }
bool ModbusRTU::Ists(uint16_t offset) { return ISTS_BOOL(Reg(ISTS(offset))); }
uint16_t ModbusRTU::Ireg(uint16_t offset) { return Reg(IREG(offset)); }
bool ModbusRTU::removeCoil(uint16_t offset, uint16_t registerCount) { return removeReg(COIL(offset), registerCount); }
bool ModbusRTU::removeIsts(uint16_t offset, uint16_t registerCount) { return removeReg(ISTS(offset), registerCount); }
bool ModbusRTU::removeIreg(uint16_t offset, uint16_t registerCount) { return removeReg(IREG(offset), registerCount); }
bool ModbusRTU::onGetCoil(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return onGet(COIL(offset), callbackFunction, registerCount);
}
bool ModbusRTU::onSetCoil(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return onSet(COIL(offset), callbackFunction, registerCount);
}
bool ModbusRTU::onGetHreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return onGet(HREG(offset), callbackFunction, registerCount);
}
bool ModbusRTU::onSetHreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return onSet(HREG(offset), callbackFunction, registerCount);
}
bool ModbusRTU::onGetIsts(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return onGet(ISTS(offset), callbackFunction, registerCount);
}
bool ModbusRTU::onSetIsts(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return onSet(ISTS(offset), callbackFunction, registerCount);
}
bool ModbusRTU::onGetIreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return onGet(IREG(offset), callbackFunction, registerCount);
}
bool ModbusRTU::onSetIreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return onSet(IREG(offset), callbackFunction, registerCount);
}
bool ModbusRTU::removeOnGetCoil(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOnGet(COIL(offset), callbackFunction, registerCount);
}
bool ModbusRTU::removeOnSetCoil(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOnSet(COIL(offset), callbackFunction, registerCount);
}
bool ModbusRTU::removeOnGetHreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOnGet(HREG(offset), callbackFunction, registerCount);
}
bool ModbusRTU::removeOnSetHreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOnSet(HREG(offset), callbackFunction, registerCount);
}
bool ModbusRTU::removeOnGetIsts(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOnGet(ISTS(offset), callbackFunction, registerCount);
}
bool ModbusRTU::removeOnSetIsts(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOnSet(ISTS(offset), callbackFunction, registerCount);
}
bool ModbusRTU::removeOnGetIreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOnGet(IREG(offset), callbackFunction, registerCount);
}
bool ModbusRTU::removeOnSetIreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return removeOnSet(IREG(offset), callbackFunction, registerCount);
}
uint16_t ModbusRTU::readFileRec(uint8_t slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction) {
  if (!data || !len || startRec > 0x270F) return 0;
  if (!readSlaveFile(&fileNum, &startRec, &len, 1, Modbus::FC_READ_FILE_REC)) return 0;
  return send(slaveId, HREG(0), callbackFunction, data); // HREG(0) - just dummy value
};
uint16_t ModbusRTU::writeFileRec(uint8_t slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction) {
  if (!data || !len || startRec > 0x270F) return 0;
  if (!writeSlaveFile(&fileNum, &startRec, &len, 1, Modbus::FC_WRITE_FILE_REC, data)) return 0;
  return send(slaveId, HREG(0), callbackFunction); // HREG(0) - just dummy value
};
uint16_t ModbusRTU::maskHreg(uint8_t slaveId, uint16_t offset, uint16_t andMask, uint16_t orMask, cbTransaction callbackFunction) {
  _len = 7;
  _frame[0] = Modbus::FC_MASKWRITE_REG;
  _frame[1] = offset >> 8;
  _frame[2] = offset & 0x00FF;
  _frame[3] = andMask >> 8;
  _frame[4] = andMask & 0x00FF;
  _frame[5] = orMask >> 8;
  _frame[6] = orMask & 0x00FF;
  return send(slaveId, HREG(offset), callbackFunction);
};

uint16_t ModbusRTU::readWriteHreg(uint8_t target, uint16_t readOffset, uint16_t* readValue, uint16_t readRegisterCount, uint16_t writeOffset,
                                  uint16_t* writeValue, uint16_t writeRegisterCount, cbTransaction callbackFunction) {
  const uint8_t headerLength = 10;
  if (readRegisterCount < 0x0001 || readRegisterCount > MODBUS_MAX_WORDS || uint32_t(readOffset) + readRegisterCount > 65536 ||
      uint32_t(writeOffset) + writeRegisterCount > 65536 || writeRegisterCount < 0x0001 || writeRegisterCount > 0X0079 || !readValue || !writeValue)
    return 0;
  _len = headerLength + 2 * writeRegisterCount;
  _frame[0] = Modbus::FC_READWRITE_REGS;
  _frame[1] = readOffset >> 8;
  _frame[2] = readOffset & 0x00FF;
  _frame[3] = readRegisterCount >> 8;
  _frame[4] = readRegisterCount & 0x00FF;

  _frame[5] = writeOffset >> 8;
  _frame[6] = writeOffset & 0x00FF;
  _frame[7] = writeRegisterCount >> 8;
  _frame[8] = writeRegisterCount & 0x00FF;
  _frame[9] = _len - headerLength;

  uint8_t* frame = _frame + headerLength;
  for (uint8_t index = 0; index < writeRegisterCount; index++) {
    modbus_detail::writeWord(frame + 2 * index, writeValue[index]);
  }
  return send(target, HREG(readOffset), callbackFunction, (uint8_t*)readValue);
};

uint16_t ModbusRTU::rawRequest(uint8_t target, uint8_t* data, uint16_t len, cbTransaction callbackFunction) {
  if (!data || !len || len > MODBUS_MAX_FRAME) return 0;
  _len = len;
  memcpy(_frame, data, len);
  return send(target, HREG(0), callbackFunction);
};

uint16_t ModbusRTU::rawResponse(uint8_t target, uint8_t* data, uint16_t len) {
  if (!data || !len || len > MODBUS_MAX_FRAME) return 0;
  _len = len;
  memcpy(_frame, data, len);
  return send(target, HREG(0), nullptr, nullptr, false);
};

uint16_t ModbusRTU::errorResponse(uint8_t target, Modbus::FunctionCode functionCode, Modbus::ResultCode exceptionCode) {
  exceptionResponse(functionCode, exceptionCode);
  return send(target, HREG(0), nullptr, nullptr, false);
}
