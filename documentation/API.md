# Common API

```c
void task();
```

Call this processing routine periodically from `loop()`.

## Server API

### Select TCP or UDP mode

```c
void setMode(ModbusTransportMode mode = MODE_TCP);
```

### Add registers

```c
bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t registerCount = 1);
bool addCoil(uint16_t offset, bool value = false, uint16_t registerCount = 1);
bool addIsts(uint16_t offset, bool value = false, uint16_t registerCount = 1);
bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t registerCount = 1);
```

### Write a local register

```c
bool Hreg(uint16_t offset, uint16_t value);
bool Coil(uint16_t offset, bool value);
bool Ists(uint16_t offset, bool value);
bool Ireg(uint16_t offset, uint16_t value);
```

### Read a local register

```c
uint16_t Hreg(uint16_t offset);
bool Coil(uint16_t offset);
bool Ists(uint16_t offset);
uint16_t Ireg(uint16_t offset);
```

### Remove a local register

```c
bool removeHreg(uint16_t offset, uint16_t registerCount = 1);
bool removeCoil(uint16_t offset, uint16_t registerCount = 1);
bool removeIsts(uint16_t offset, uint16_t registerCount = 1);
bool removeIreg(uint16_t offset, uint16_t registerCount = 1);
```

### Modbus RTU Specific API

```c
bool begin(SoftwareSerial* port, int16_t txPin = -1, bool direct = true);
bool begin(HardwareSerial* port, int16_t txPin = -1, bool direct = true);
bool begin(Stream* port);
```

Assign the serial port. `txPin` controls transmit enable for a MAX485 transceiver. Pass `direct = false` if `txPin` uses inverted logic.

```c
void setBaudrate(uint32_t baud);
```

Configure the RTU timing for the serial baud rate. On non-ESP devices, call this method after `begin()`.

```c
void server(uint8_t slaveId);
void slave(uint8_t slaveId); // Deprecated.
```

Select and initialize client or server mode. Switching modes after initialization is unsupported and produces undefined behavior.

```c
uint8_t server();
uint8_t slave(); // Deprecated.
```

In server mode, return the configured server identifier. In client mode, return the server identifier for the active request, or zero when no request
is in progress.

## Modbus TCP server-specific API

```c
void begin(); // Deprecated. Use server() instead.
void slave(uint16_t port = MODBUSIP_PORT); // Compatibility alias for server().
void server(uint16_t port = MODBUSIP_PORT);
```

## Modbus TCP client-specific API

```c
void master(); // Compatibility alias for client().
void client();
bool connect(IPAddress ipAddress, uint16_t port = MODBUSIP_PORT);
bool disconnect(IPAddress ipAddress);
bool isTransaction(uint16_t transactionIdentifier);
bool isConnected(IPAddress ipAddress);
void dropTransactions();
```

```c
void autoConnect(bool enabled);
```

Control automatic connection behavior for read, write, pull, and push calls. When disabled (the default), an operation fails if no connection to the
server exists. When enabled, the operation attempts to connect before sending the request.

## Client API

### Read coils (0x01) from a server

```c
uint16_t readCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t readCoil(IPAddress ipAddress, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t readCoil(const char* host, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t readCoil(String host, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

uint16_t pullCoil(uint8_t slaveId, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t pullCoilToIsts(uint8_t slaveId, uint16_t offset, uint16_t startRegister, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t pullCoil(IPAddress ipAddress, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t pullCoilToIsts(IPAddress ipAddress, uint16_t offset, uint16_t startRegister, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
```

### Write a single coil to a server

```c
uint16_t writeCoil(IPAddress ipAddress, uint16_t offset, bool value, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t writeCoil(uint8_t slaveId, uint16_t offset, bool value, cbTransaction callbackFunction = nullptr);
uint16_t writeCoil(IPAddress ipAddress, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t writeCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);

uint16_t pushCoil(uint8_t slaveId, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t pushCoil(IPAddress ipAddress, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
```

### Read discrete inputs (0x02) from a server

```c
uint16_t readIsts(uint8_t slaveId, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t readIsts(IPAddress ipAddress, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

uint16_t pullIsts(uint8_t slaveId, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t pushIstsToCoil(uint8_t slaveId, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);

uint16_t pullIsts(IPAddress ipAddress, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t pushIstsToCoil(IPAddress ipAddress, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
```

```c
uint16_t pullHreg(IPAddress ipAddress, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t pullIreg(IPAddress ipAddress, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t pullHregToIreg(IPAddress ipAddress, uint16_t offset, uint16_t startRegister, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t pullHreg(uint8_t slaveId, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t pullIreg(uint8_t slaveId, uint16_t from, uint16_t destination, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t pullHregToIreg(uint8_t slaveId, uint16_t offset, uint16_t startRegister, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
```

The result is stored in local registers. The method returns the corresponding transaction identifier. `ipAddress` and `from` or `offset` identify the
remote server and register; `destination` or `startRegister` identifies the first local register.

### Send one or more registers to a remote server

```c
uint16_t pushHreg(IPAddress ipAddress, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t pushIregToHreg(IPAddress ipAddress, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

uint16_t pushHreg(uint8_t slaveId, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t pushIregToHreg(uint8_t slaveId, uint16_t destination, uint16_t from, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
```

The library selects the single- or multiple-register Modbus function according to `registerCount`. `ipAddress` and `destination` identify the remote
server and register; `from` identifies the first local register.

### Write one or more values to remote server registers

```c
uint16_t writeHreg(IPAddress ipAddress, uint16_t offset, uint16_t value, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t writeHreg(uint8_t slaveId, uint16_t offset, uint16_t value, cbTransaction callbackFunction = nullptr);
```

Write a single value to a remote holding register or coil.

```c
uint16_t writeHreg(IPAddress ipAddress, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t writeHreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
```

Write multiple values from an array to remote coils or holding registers.

### Read values from multiple remote server registers

```c
uint16_t readHreg(IPAddress ipAddress, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t readIreg(IPAddress ipAddress, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);

uint16_t readHreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
uint16_t readIreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr);
```

Read remote holding registers, coils, input registers, or discrete inputs into an array.

## Callbacks API

```c
void cbEnable(bool state = true);
void cbDisable();
```

Enable or disable callback generation. Callbacks are enabled by default. _This setting does not affect transaction callbacks._

```c
void onConnect(cbModbusConnect callbackFunction);
void onDisconnect(cbModbusConnect callbackFunction);
```

_Modbus TCP server:_ assign a callback for new incoming connections.

```c
typedef bool (*cbModbusConnect)(IPAddress ipAddress);
```

The connect callback receives the client's IP address. The disconnect callback always receives `INADDR_NONE`.

```c
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t value);
```

Register get/set callbacks receive a pointer to the register's `TRegister` structure and its new value.

```c
typedef bool (*cbTransaction)(Modbus::ResultCode event, uint16_t transactionId, void* data);
```

The transaction-completion callback receives the result code, transaction identifier, and data pointer. For Modbus TCP, `data` is currently reserved;
for Modbus RTU, `transactionId` is also reserved.

```c
uint32_t eventSource();
```

Call this method from an `onGet`, `onSet`, or transaction callback.

_Modbus TCP client/server:_ return the remote requester's IP address, or `INADDR_NONE` for a local operation. Construct `IPAddress(eventSource())` to
use the result as an `IPAddress`.

_Note:_ A transaction callback receives `INADDR_NONE` when the transaction times out.

_Modbus RTU client/server:_ return the server identifier.

```c
bool onSetCoil(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onSetHreg(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onSetIsts(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onSetIreg(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
```

Assign a callback for register-modification events. Use `registerCount` to assign it to multiple sequential registers.

```c
bool onGetCoil(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onGetHreg(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onGetIsts(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
bool onGetIreg(uint16_t address, cbModbus callbackFunction = nullptr, uint16_t registerCount = 1);
```

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

Remove a specific callback, or all callbacks of that type when `callbackFunction == nullptr`.

### Macros

```c
#define COIL_VAL(value)
#define COIL_BOOL(value)
#define ISTS_VAL(value)
#define ISTS_BOOL(value)
```

## Contributions

https://github.com/emelianov/modbus-esp8266

a.m.emelianov@gmail.com

Original version:

https://github.com/andresarmento/modbus-esp8266 https://github.com/andresarmento/modbus-arduino

prof (at) andresarmento (dot) com

## License

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
