# ESP8266/ESP32 TCP Examples

## [Basic client](client.ino)

## [Client with blocking read operation](clientSync.ino)

## [Server](server.ino)

### API

```c
void client();
```

Initialize internal structures to act as a Modbus client.

```c
bool connect(IPAddress ipAddress, uint16_t port = MODBUSIP_PORT);
bool disconnect(IPAddress ipAddress);
```

- `ipAddress`: IP address of the remote Modbus server
- `port`: TCP port of the remote Modbus server; the standard value is 502

Only one connection to a specific address is supported. The library cannot connect simultaneously to servers that have the same IP address but use
different ports.

```c
uint16_t readCoil(IPAddress ipAddress, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t readCoil(const char* host, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t readCoil(String host, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t writeCoil(IPAddress ipAddress, uint16_t offset, bool value, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t writeCoil(IPAddress ipAddress, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t readIsts(IPAddress ipAddress, uint16_t offset, bool* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t writeHreg(IPAddress ipAddress, uint16_t offset, uint16_t value, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t writeHreg(IPAddress ipAddress, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t readHreg(IPAddress ipAddress, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
uint16_t readIreg(IPAddress ipAddress, uint16_t offset, uint16_t* value, uint16_t registerCount = 1, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
```

- `ipAddress`: IP address of the Modbus server
- `host`: host name of the Modbus server
- `offset`: address of the first Modbus register to read or write
- `registerCount`: number of registers to read or write
- `callbackFunction`: transaction callback; see the [callback examples](../Callback), or pass `nullptr` when unused
- `unit`: Modbus unit identifier

These methods send the corresponding request to the Modbus server. Unless automatic connection is enabled, call `connect(ipAddress)` first. A method
returns its transaction identifier, or zero if it cannot send the request. Read and write methods return immediately; `value` is populated only after
the response arrives and `task()` processes it.

```c
bool isTransaction(uint16_t transactionIdentifier);
```

- `transactionIdentifier`: transaction identifier

Returns `true` while the transaction is active.

```c
bool isConnected(IPAddress ipAddress);
```

- `ipAddress` Remote Modbus server IP address

Returns `true` when a connection to the server at `ipAddress` is established.

```c
void dropTransactions();
```

Cancel all active transactions. If assigned, each transaction callback receives `Modbus::EX_CANCEL`.

### Add local register

```c
bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t registerCount = 1);
bool addCoil(uint16_t offset, bool value = false, uint16_t registerCount = 1);
bool addIsts(uint16_t offset, bool value = false, uint16_t registerCount = 1);
bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t registerCount = 1);
```

- `offset` Address of the first register to add
- `value` Initial value to be assigned to register(s)
- `registerCount` Count of registers to be created

Add new registers and assign their values. Existing registers in the range are updated. Returns `true` on success.

### Write local reg

```c
bool Hreg(uint16_t offset, uint16_t value);
bool Coil(uint16_t offset, bool value);
bool Ists(uint16_t offset, bool value);
bool Ireg(uint16_t offset, uint16_t value);
```

- `offset` Address of the register
- `value` Value to be assigned to register

Returns `true` on success, or `false` if the register was not previously added or another error occurs.

### Read local reg

```c
uint16_t Hreg(uint16_t offset);
bool Coil(uint16_t offset);
bool Ists(uint16_t offset);
uint16_t Ireg(uint16_t offset);
```

- `offset` Address of the register to read

Returns current value of the register.

### Remove reg(s)

```c
bool removeHreg(uint16_t offset, uint16_t registerCount = 1);
bool removeCoil(uint16_t offset, uint16_t registerCount = 1);
bool removeIsts(uint16_t offset, uint16_t registerCount = 1);
bool removeIreg(uint16_t offset, uint16_t registerCount = 1);
```

- `offset` Address of the first register to remove
- `registerCount` Count of registers to be created

Remove `registerCount` registers starting at `offset`. Missing registers in the range are skipped. Returns `true` if at least one register in the
range was removed.

# Modbus Library for Arduino

### ModbusRTU, ModbusTCP and ModbusTCP Security

(c)2020 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
