/*
    Modbus Library for Arduino
        Modbus public API implementation
    Copyright (C) 2014 André Sarmento Barbosa
                  2017-2021 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once
#include "Modbus.h"

template <class Transport> class ModbusAPI : public Transport {
public:
  /*	// New API
          template <typename TYPEID>
          uint16_t write(TYPEID target, TAddress reg, uint16_t value, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
          template <typename TYPEID>
          uint16_t write(TYPEID target, TAddress reg, bool value, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
          template <typename TYPEID>
          uint16_t write(TYPEID target, TAddress reg, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
          template <typename TYPEID>
          uint16_t write(TYPEID target, TAddress reg, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
          template <typename TYPEID>
          uint16_t read(TYPEID target, TAddress reg, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
          template <typename TYPEID>
          uint16_t read(TYPEID target, TAddress reg, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

          template <typename TYPEID>
          uint16_t push(TYPEID target, TAddress destination, TAddress from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
          template <typename TYPEID>
          uint16_t pull(TYPEID target, TAddress from, TAddress destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  */
  // Legacy API
  bool Hregs(uint16_t offset, uint16_t* value, uint16_t registerCount = 1) { return this->Reg(HREG(offset), value, registerCount); }
  bool Coils(uint16_t offset, bool* value, uint16_t registerCount = 1) { return this->Reg(COIL(offset), value, registerCount); }
  bool Istss(uint16_t offset, bool* value, uint16_t registerCount = 1) { return this->Reg(ISTS(offset), value, registerCount); }
  bool Iregs(uint16_t offset, uint16_t* value, uint16_t registerCount = 1) { return this->Reg(IREG(offset), value, registerCount); }

  // bool addHreg(uint16_t offset, uint16_t* value, uint16_t registerCount = 1) {return this->addReg(HREG(offset), value);}
  // bool addCoil(uint16_t offset, bool* value, uint16_t registerCount = 1) {return this->addReg(COIL(offset), value);}
  // bool addIsts(uint16_t offset, bool* value, uint16_t registerCount = 1) {return this->addReg(ISTS(offset), value);}
  // bool addIreg(uint16_t offset, uint16_t* value, uint16_t registerCount = 1) {return this->addReg(IREG(offset), value);}

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

  bool onGetCoil(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSetCoil(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onGetHreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSetHreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onGetIsts(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSetIsts(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onGetIreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool onSetIreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);

  bool removeOnGetCoil(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSetCoil(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnGetHreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSetHreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnGetIsts(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSetIsts(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnGetIreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
  bool removeOnSetIreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);

  template <typename TYPEID> uint16_t writeCoil(TYPEID target, uint16_t offset, bool value, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t writeCoil(TYPEID target, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t readCoil(TYPEID target, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID> uint16_t writeHreg(TYPEID target, uint16_t offset, uint16_t value, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t writeHreg(TYPEID target, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t readIsts(TYPEID target, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t readHreg(TYPEID target, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t readIreg(TYPEID target, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

  template <typename TYPEID>
  uint16_t pushCoil(TYPEID target, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t pullCoil(TYPEID target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t pullIsts(TYPEID target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t pushHreg(TYPEID target, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t pullHreg(TYPEID target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t pullIreg(TYPEID target, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

  template <typename TYPEID>
  uint16_t
  pullHregToIreg(TYPEID target, uint16_t offset, uint16_t startRegister, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t
  pullCoilToIsts(TYPEID target, uint16_t offset, uint16_t startRegister, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t pushIstsToCoil(TYPEID target, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t pushIregToHreg(TYPEID target, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

  template <typename TYPEID>
  uint16_t readFileRec(TYPEID slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction = nullptr,
                       uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t writeFileRec(TYPEID slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction = nullptr,
                        uint8_t unit = MODBUSIP_UNIT);

  template <typename TYPEID>
  uint16_t maskHreg(TYPEID slaveId, uint16_t offset, uint16_t andMask, uint16_t orMask, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t readWriteHreg(TYPEID slaveId, uint16_t readOffset, uint16_t* readValue, uint16_t readRegisterCount, uint16_t writeOffset, uint16_t* writeValue,
                         uint16_t writeRegisterCount, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

  template <typename TYPEID>
  uint16_t rawRequest(TYPEID target, uint8_t* data, uint16_t len, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID> uint16_t rawResponse(TYPEID target, uint8_t* data, uint16_t len, uint8_t unit = MODBUSIP_UNIT);
  template <typename TYPEID>
  uint16_t errorResponse(TYPEID target, Modbus::FunctionCode functionCode, Modbus::ResultCode exceptionCode, uint8_t unit = MODBUSIP_UNIT);

  // Deprecated misspellings retained for source compatibility.
  template <typename TYPEID> uint16_t rawResponce(TYPEID target, uint8_t* data, uint16_t len, uint8_t unit = MODBUSIP_UNIT) {
    return rawResponse(target, data, len, unit);
  }
  template <typename TYPEID>
  uint16_t errorResponce(TYPEID target, Modbus::FunctionCode functionCode, Modbus::ResultCode exceptionCode, uint8_t unit = MODBUSIP_UNIT) {
    return errorResponse(target, functionCode, exceptionCode, unit);
  }
};

// FNAME	writeCoil, writeIsts, writeHreg, writeIreg
// REG		COIL, ISTS, HREG, IREG
// FUNC		Modbus function
// MAXNUM	Register count limit
// VALTYPE	bool, uint16_t
// VALUE
#define IMPLEMENT_WRITEREG(FNAME, REG, FUNC, VALUE, VALTYPE)                                                                                         \
  template <class Transport>                                                                                                                         \
  template <typename TYPEID>                                                                                                                         \
  uint16_t ModbusAPI<Transport>::FNAME(TYPEID target, uint16_t offset, VALTYPE value, cbTransaction callbackFunction, uint8_t unit) {                \
    this->readSlave(offset, VALUE(value), Modbus::FUNC);                                                                                             \
    return this->send(target, REG(offset), callbackFunction, unit);                                                                                  \
  }
IMPLEMENT_WRITEREG(writeCoil, COIL, FC_WRITE_COIL, COIL_VAL, bool)
IMPLEMENT_WRITEREG(writeHreg, HREG, FC_WRITE_REG, , uint16_t)

#define IMPLEMENT_WRITEREGS(FNAME, REG, FUNC, VALUE, MAXNUM, VALTYPE)                                                                                \
  template <class Transport>                                                                                                                         \
  template <typename TYPEID>                                                                                                                         \
  uint16_t ModbusAPI<Transport>::FNAME(TYPEID target, uint16_t offset, VALTYPE* value, uint16_t registerCount, cbTransaction callbackFunction,       \
                                       uint8_t unit) {                                                                                               \
    if (registerCount < 0x0001 || registerCount > MAXNUM) return false;                                                                              \
    this->VALUE(REG(offset), offset, registerCount, Modbus::FUNC, value);                                                                            \
    return this->send(target, REG(offset), callbackFunction, unit);                                                                                  \
  }
IMPLEMENT_WRITEREGS(writeCoil, COIL, FC_WRITE_COILS, writeSlaveBits, 0x07D0, bool)
IMPLEMENT_WRITEREGS(writeHreg, HREG, FC_WRITE_REGS, writeSlaveWords, 0x007D, uint16_t)

#define IMPLEMENT_READREGS(FNAME, REG, FUNC, MAXNUM, VALTYPE)                                                                                        \
  template <class Transport>                                                                                                                         \
  template <typename TYPEID>                                                                                                                         \
  uint16_t ModbusAPI<Transport>::FNAME(TYPEID target, uint16_t offset, VALTYPE* value, uint16_t registerCount, cbTransaction callbackFunction,       \
                                       uint8_t unit) {                                                                                               \
    if (registerCount < 0x0001 || registerCount > MAXNUM) return false;                                                                              \
    this->readSlave(offset, registerCount, Modbus::FUNC);                                                                                            \
    return this->send(target, REG(offset), callbackFunction, unit, (uint8_t*)value);                                                                 \
  }
IMPLEMENT_READREGS(readCoil, COIL, FC_READ_COILS, 0x07D0, bool)
IMPLEMENT_READREGS(readHreg, HREG, FC_READ_REGS, 0x007D, uint16_t)
IMPLEMENT_READREGS(readIsts, ISTS, FC_READ_INPUT_STAT, 0x07D0, bool)
IMPLEMENT_READREGS(readIreg, IREG, FC_READ_INPUT_REGS, 0x007D, uint16_t)

#if defined(MODBUS_ADD_REG)
#define ADDREG(REGISTER_TYPE) this->addReg(REGISTER_TYPE(destination), (uint16_t)0, registerCount);
#else
#define ADDREG(REGISTER_TYPE) ;
#endif
#define IMPLEMENT_PULL(FNAME, REG, FUNC, MAXNUM)                                                                                                     \
  template <class Transport>                                                                                                                         \
  template <typename TYPEID>                                                                                                                         \
  uint16_t ModbusAPI<Transport>::FNAME(TYPEID target, uint16_t from, uint16_t destination, uint16_t registerCount, cbTransaction callbackFunction,   \
                                       uint8_t unit) {                                                                                               \
    if (registerCount < 0x0001 || registerCount > MAXNUM) return false;                                                                              \
    ADDREG(REG)                                                                                                                                      \
    this->readSlave(from, registerCount, Modbus::FUNC);                                                                                              \
    return this->send(target, REG(destination), callbackFunction, unit);                                                                             \
  }
IMPLEMENT_PULL(pullCoil, COIL, FC_READ_COILS, 0x07D0)
IMPLEMENT_PULL(pullIsts, ISTS, FC_READ_INPUT_STAT, 0x07D0)
IMPLEMENT_PULL(pullHreg, HREG, FC_READ_REGS, 0x007D)
IMPLEMENT_PULL(pullIreg, IREG, FC_READ_INPUT_REGS, 0x007D)
IMPLEMENT_PULL(pullHregToIreg, IREG, FC_READ_REGS, 0x007D)
IMPLEMENT_PULL(pullCoilToIsts, ISTS, FC_READ_COILS, 0x07D0)

#define IMPLEMENT_PUSH(FNAME, REG, FUNC, MAXNUM, FINT)                                                                                               \
  template <class Transport>                                                                                                                         \
  template <typename TYPEID>                                                                                                                         \
  uint16_t ModbusAPI<Transport>::FNAME(TYPEID target, uint16_t destination, uint16_t from, uint16_t registerCount, cbTransaction callbackFunction,   \
                                       uint8_t unit) {                                                                                               \
    if (registerCount < 0x0001 || registerCount > MAXNUM) return false;                                                                              \
    if (!this->searchRegister(REG(from))) return false;                                                                                              \
    this->FINT(REG(from), destination, registerCount, Modbus::FUNC);                                                                                 \
    return this->send(target, REG(from), callbackFunction, unit);                                                                                    \
  }
IMPLEMENT_PUSH(pushCoil, COIL, FC_WRITE_COILS, 0x7D0, writeSlaveBits)
IMPLEMENT_PUSH(pushHreg, HREG, FC_WRITE_REGS, 0x007D, writeSlaveWords)
IMPLEMENT_PUSH(pushIregToHreg, IREG, FC_WRITE_REGS, 0x007D, writeSlaveWords)
IMPLEMENT_PUSH(pushIstsToCoil, ISTS, FC_WRITE_COILS, 0x07D0, writeSlaveBits)

template <class Transport> bool ModbusAPI<Transport>::addHreg(uint16_t offset, uint16_t value, uint16_t registerCount) {
  return this->addReg(HREG(offset), value, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::Hreg(uint16_t offset, uint16_t value) { return this->Reg(HREG(offset), value); }
template <class Transport> uint16_t ModbusAPI<Transport>::Hreg(uint16_t offset) { return this->Reg(HREG(offset)); }
template <class Transport> bool ModbusAPI<Transport>::removeHreg(uint16_t offset, uint16_t registerCount) { return this->removeReg(HREG(offset), registerCount); }
template <class Transport> bool ModbusAPI<Transport>::addCoil(uint16_t offset, bool value, uint16_t registerCount) {
  return this->addReg(COIL(offset), COIL_VAL(value), registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::addIsts(uint16_t offset, bool value, uint16_t registerCount) {
  return this->addReg(ISTS(offset), ISTS_VAL(value), registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::addIreg(uint16_t offset, uint16_t value, uint16_t registerCount) {
  return this->addReg(IREG(offset), value, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::Coil(uint16_t offset, bool value) { return this->Reg(COIL(offset), COIL_VAL(value)); }
template <class Transport> bool ModbusAPI<Transport>::Ists(uint16_t offset, bool value) { return this->Reg(ISTS(offset), ISTS_VAL(value)); }
template <class Transport> bool ModbusAPI<Transport>::Ireg(uint16_t offset, uint16_t value) { return this->Reg(IREG(offset), value); }
template <class Transport> bool ModbusAPI<Transport>::Coil(uint16_t offset) { return COIL_BOOL(this->Reg(COIL(offset))); }
template <class Transport> bool ModbusAPI<Transport>::Ists(uint16_t offset) { return ISTS_BOOL(this->Reg(ISTS(offset))); }
template <class Transport> uint16_t ModbusAPI<Transport>::Ireg(uint16_t offset) { return this->Reg(IREG(offset)); }
template <class Transport> bool ModbusAPI<Transport>::removeCoil(uint16_t offset, uint16_t registerCount) { return this->removeReg(COIL(offset), registerCount); }
template <class Transport> bool ModbusAPI<Transport>::removeIsts(uint16_t offset, uint16_t registerCount) { return this->removeReg(ISTS(offset), registerCount); }
template <class Transport> bool ModbusAPI<Transport>::removeIreg(uint16_t offset, uint16_t registerCount) { return this->removeReg(IREG(offset), registerCount); }
template <class Transport> bool ModbusAPI<Transport>::onGetCoil(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) { return this->onGet(COIL(offset), callbackFunction, registerCount); }
template <class Transport> bool ModbusAPI<Transport>::onSetCoil(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) { return this->onSet(COIL(offset), callbackFunction, registerCount); }
template <class Transport> bool ModbusAPI<Transport>::onGetHreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) { return this->onGet(HREG(offset), callbackFunction, registerCount); }
template <class Transport> bool ModbusAPI<Transport>::onSetHreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) { return this->onSet(HREG(offset), callbackFunction, registerCount); }
template <class Transport> bool ModbusAPI<Transport>::onGetIsts(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) { return this->onGet(ISTS(offset), callbackFunction, registerCount); }
template <class Transport> bool ModbusAPI<Transport>::onSetIsts(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) { return this->onSet(ISTS(offset), callbackFunction, registerCount); }
template <class Transport> bool ModbusAPI<Transport>::onGetIreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) { return this->onGet(IREG(offset), callbackFunction, registerCount); }
template <class Transport> bool ModbusAPI<Transport>::onSetIreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) { return this->onSet(IREG(offset), callbackFunction, registerCount); }
template <class Transport> bool ModbusAPI<Transport>::removeOnGetCoil(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return this->removeOnGet(COIL(offset), callbackFunction, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::removeOnSetCoil(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return this->removeOnSet(COIL(offset), callbackFunction, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::removeOnGetHreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return this->removeOnGet(HREG(offset), callbackFunction, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::removeOnSetHreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return this->removeOnSet(HREG(offset), callbackFunction, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::removeOnGetIsts(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return this->removeOnGet(ISTS(offset), callbackFunction, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::removeOnSetIsts(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return this->removeOnSet(ISTS(offset), callbackFunction, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::removeOnGetIreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return this->removeOnGet(IREG(offset), callbackFunction, registerCount);
}
template <class Transport> bool ModbusAPI<Transport>::removeOnSetIreg(uint16_t offset, cbModbus callbackFunction, uint16_t registerCount) {
  return this->removeOnSet(IREG(offset), callbackFunction, registerCount);
}
template <class Transport>
template <typename TYPEID>
uint16_t ModbusAPI<Transport>::readFileRec(TYPEID slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction, uint8_t unit) {
  if (startRec > 0x270F) return 0;
  if (!this->readSlaveFile(&fileNum, &startRec, &len, 1, Modbus::FC_READ_FILE_REC)) return 0;
  return this->send(slaveId, HREG(0), callbackFunction, unit, data); // HREG(0) - just dummy value
};
template <class Transport>
template <typename TYPEID>
uint16_t
ModbusAPI<Transport>::writeFileRec(TYPEID slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction, uint8_t unit) {
  if (startRec > 0x270F) return 0;
  if (!this->writeSlaveFile(&fileNum, &startRec, &len, 1, Modbus::FC_WRITE_FILE_REC, data)) return 0;
  return this->send(slaveId, HREG(0), callbackFunction, unit); // HREG(0) - just dummy value
};
template <class Transport>
template <typename TYPEID>
uint16_t ModbusAPI<Transport>::maskHreg(TYPEID slaveId, uint16_t offset, uint16_t andMask, uint16_t orMask, cbTransaction callbackFunction, uint8_t unit) {
  free(this->_frame);
  this->_len = 7;
  this->_frame = (uint8_t*)malloc(this->_len);
  this->_frame[0] = Modbus::FC_MASKWRITE_REG;
  this->_frame[1] = offset >> 8;
  this->_frame[2] = offset & 0x00FF;
  this->_frame[3] = andMask >> 8;
  this->_frame[4] = andMask & 0x00FF;
  this->_frame[5] = orMask >> 8;
  this->_frame[6] = orMask & 0x00FF;
  return this->send(slaveId, HREG(offset), callbackFunction, unit);
};

template <class Transport>
template <typename TYPEID>
uint16_t ModbusAPI<Transport>::readWriteHreg(TYPEID target, uint16_t readOffset, uint16_t* readValue, uint16_t readRegisterCount, uint16_t writeOffset,
                                     uint16_t* writeValue, uint16_t writeRegisterCount, cbTransaction callbackFunction, uint8_t unit) {
  const uint8_t headerLength = 10;
  if (readRegisterCount < 0x0001 || readRegisterCount > MODBUS_MAX_WORDS || writeRegisterCount < 0x0001 || writeRegisterCount > 0X0079 || !readValue || !writeValue) return 0;

  free(this->_frame);
  this->_len = headerLength + 2 * writeRegisterCount;
  this->_frame = (uint8_t*)malloc(this->_len);
  if (!this->_frame) {
    this->_reply = Modbus::REPLY_OFF;
    return 0;
  }
  this->_frame[0] = Modbus::FC_READWRITE_REGS;
  this->_frame[1] = readOffset >> 8;
  this->_frame[2] = readOffset & 0x00FF;
  this->_frame[3] = readRegisterCount >> 8;
  this->_frame[4] = readRegisterCount & 0x00FF;

  this->_frame[5] = writeOffset >> 8;
  this->_frame[6] = writeOffset & 0x00FF;
  this->_frame[7] = writeRegisterCount >> 8;
  this->_frame[8] = writeRegisterCount & 0x00FF;
  this->_frame[9] = this->_len - headerLength;

  uint16_t* frame = (uint16_t*)(this->_frame + headerLength);
  for (uint8_t index = 0; index < writeRegisterCount; index++) {
    frame[index] = __swap_16(writeValue[index]);
  }
  return this->send(target, HREG(readOffset), callbackFunction, unit, (uint8_t*)readValue);
};

template <class Transport>
template <typename TYPEID>
uint16_t ModbusAPI<Transport>::rawRequest(TYPEID target, uint8_t* data, uint16_t len, cbTransaction callbackFunction, uint8_t unit) {
  free(this->_frame);
  this->_frame = (uint8_t*)malloc(len);
  if (!this->_frame) return 0;
  this->_len = len;
  memcpy(this->_frame, data, len);
  return this->send(target, HREG(0), callbackFunction, unit);
};

template <class Transport> template <typename TYPEID> uint16_t ModbusAPI<Transport>::rawResponse(TYPEID target, uint8_t* data, uint16_t len, uint8_t unit) {
  free(this->_frame);
  this->_frame = (uint8_t*)malloc(len);
  if (!this->_frame) return 0;
  this->_len = len;
  memcpy(this->_frame, data, len);
  return this->send(target, HREG(0), nullptr, unit, nullptr, false);
};

template <class Transport>
template <typename TYPEID>
uint16_t ModbusAPI<Transport>::errorResponse(TYPEID target, Modbus::FunctionCode functionCode, Modbus::ResultCode exceptionCode, uint8_t unit) {
  this->exceptionResponse(functionCode, exceptionCode);
  return this->send(target, HREG(0), nullptr, unit, nullptr, false);
}
