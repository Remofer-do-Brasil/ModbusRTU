# Callbacks

## [Register read/write callback](onSet/onSet.ino)

## [Use one callback function for multiple registers](onGetShared/onGetShared.ino)

## [Incoming request callback (applicable to server/slave)](Request/Request.ino)

## [Modbus TCP/TLS Incoming connection callback](onSet/onSet.ino)

## [Modbus TCP/TLS Transaction result](Transactional/Transactional.ino)

### Callback API

```c
bool onSetCoil(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onSetHreg(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onSetIsts(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onSetIreg(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
```

- `address`: address of the first register
- `callbackFunction`: callback function
- `registerCount`: number of sequential registers to which the callback applies

Assign a callback for register-modification events. Use `registerCount` to assign it to multiple sequential registers.

```c
bool onGetCoil(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onGetHreg(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onGetIsts(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onGetIreg(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
```

- `address`: address of the first register
- `callbackFunction`: callback function
- `registerCount`: number of sequential registers to which the callback applies

Assign a callback for register-query events. Use `registerCount` to assign it to multiple sequential registers.

```c
bool removeOnGetCoil(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool removeOnSetCoil(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool removeOnGetHreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool removeOnSetHreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool removeOnGetIsts(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool removeOnSetIsts(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool removeOnGetIreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool removeOnSetIreg(uint16_t offset, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
```

- `offset`: address of the first register
- `callbackFunction`: callback to remove, or `nullptr` to remove all matching callbacks
- `registerCount`: number of sequential registers from which to remove the callback

Remove a specific callback, or all callbacks of that type when `callbackFunction == nullptr`.

```c
typedef Modbus::ResultCode (*cbRequest)(Modbus::FunctionCode functionCode, const Modbus::RequestData data);
bool onRequest(cbRequest callbackFunction = _onRequestDefault);
bool onRequestSuccess(cbRequest callbackFunction = _onRequestDefault);

union Modbus::RequestData {
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
};
```

The callback receives the Modbus function code and a `Modbus::RequestData` structure containing the requested register type, offset, and count. Return
`Modbus::EX_SUCCESS` to allow processing, or a Modbus error code to reject the request and return that code to the client.

```c
void onConnect(cbModbusConnect callbackFunction);
void onDisconnect(cbModbusConnect callbackFunction);
```

Assign a callback for incoming connections.

```c
typedef bool (*cbModbusConnect)(IPAddress ipAddress);
```

- `ipAddress`: client address for a connection event; `INADDR_NONE` for a disconnect event

## Result codes _Modbus::ResultCode_

| Value                               | Hex  | Definition | Description                                           |
| ----------------------------------- | ---- | ---------- | ----------------------------------------------------- |
| Modbus::EX_SUCCESS                  | 0x00 | Custom     | No error                                              |
| Modbus::EX_ILLEGAL_FUNCTION         | 0x01 | Modbus     | Function code is not supported                        |
| Modbus::EX_ILLEGAL_ADDRESS          | 0x02 | Modbus     | Output address does not exist                         |
| Modbus::EX_ILLEGAL_VALUE            | 0x03 | Modbus     | Output value is out of range                          |
| Modbus::EX_SLAVE_FAILURE            | 0x04 | Modbus     | Server or client device failed to process the request |
| Modbus::EX_ACKNOWLEDGE              | 0x05 | Modbus     | Not used                                              |
| Modbus::EX_SLAVE_DEVICE_BUSY        | 0x06 | Modbus     | Not used                                              |
| Modbus::EX_MEMORY_PARITY_ERROR      | 0x08 | Modbus     | Not used                                              |
| Modbus::EX_PATH_UNAVAILABLE         | 0x0A | Modbus     | Not used                                              |
| Modbus::EX_DEVICE_FAILED_TO_RESPOND | 0x0B | Modbus     | Not used                                              |
| Modbus::EX_GENERAL_FAILURE          | 0xE1 | Custom     | Unexpected client error                               |
| Modbus::EX_DATA_MISMATCH            | 0xE2 | Custom     | Input data size mismatch                              |
| Modbus::EX_UNEXPECTED_RESPONSE      | 0xE3 | Custom     | Returned result does not match transaction            |
| Modbus::EX_TIMEOUT                  | 0xE4 | Custom     | Operation not finished within reasonable time         |
| Modbus::EX_CONNECTION_LOST          | 0xE5 | Custom     | Connection with device lost                           |
| Modbus::EX_CANCEL                   | 0xE6 | Custom     | Transaction/request canceled                          |

# Modbus Library for Arduino

### ModbusRTU, ModbusTCP and ModbusTCP Security

(c)2020 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
