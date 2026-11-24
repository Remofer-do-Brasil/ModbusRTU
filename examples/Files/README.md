# File operations

## [Firmware update over Modbus RTU: source node](FW-Update-Source/FW-Update-Source.ino)

Modbus RTU client that sends firmware to a server node.

To use the example:

- Connect to the target node.
- Prepare a binary image with **Sketch > Export Compiled Binary**.
- Open `http://<device-address>/` in a browser.
- Select the firmware file.
- Press **Update firmware**.
- Monitor update progress in the debug console.

## [Firmware update over Modbus RTU: target node](FW-Update-Target/FW-Update-Target.ino)

Modbus RTU server that receives and flashes new firmware.

## File block API

### Client side

```c
uint16_t readFileRec(uint8_t slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction);
uint16_t writeFileRec(uint8_t slaveId, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction);

uint16_t readFileRec(IPAddress ipAddress, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction, uint8_t unit);
uint16_t writeFileRec(IPAddress ipAddress, uint16_t fileNum, uint16_t startRec, uint16_t len, uint8_t* data, cbTransaction callbackFunction, uint8_t unit);
```

- `slaveId`: server identifier.
- `ipAddress`: server IP address.
- `fileNum`: file number.
- `startRec`: starting file offset, in words.
- `len`: data length, in words.
- `data`: data buffer. For `readFileRec`, it must contain at least `len * 2` bytes.
- `callbackFunction`: transaction callback function.
- `unit`: Modbus TCP unit identifier.

### Server side

```c
typedef std::function<Modbus::ResultCode(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)> cbModbusFileOp; // STL
typedef Modbus::ResultCode (*cbModbusFileOp)(Modbus::FunctionCode functionCode, uint16_t fileNumber, uint16_t recordNumber, uint16_t recordLength, uint8_t* frame); // Without STL

bool onFile(std::function<Modbus::ResultCode(FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)>); // STL
bool onFile(Modbus::ResultCode (*callbackFunction)(Modbus::FunctionCode, uint16_t, uint16_t, uint16_t, uint8_t*)); // Without STL
```

- `functionCode`: function code to process (`FC_READ_FILE_REC` or `FC_WRITE_FILE_REC`).
- `fileNumber`: number of the file to read or write.
- `recordNumber`: record number within the file; one record is one 2-byte word.
- `recordLength`: number of records to read or write.
- `frame`: data buffer.

`onFile` assigns the file-operation handler.

# Modbus Library for Arduino

### Modbus RTU, Modbus TCP, and Modbus TCP Security

(c)2021 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
