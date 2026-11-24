# FAQ

This library allows an Arduino board to communicate using the Modbus protocol, which is commonly used in industrial and home automation.

Modbus commonly uses RS-485 as its physical layer (Modbus Serial) or TCP/IP over Ethernet or Wi-Fi (Modbus TCP and Modbus TCP Security).

---

## Where can I find the library documentation?

- [API](API.md)
- [ModbusTCP](https://github.com/emelianov/modbus-esp8266/tree/master/examples/TCP-ESP#API)
- [ModbusRTU](https://github.com/emelianov/modbus-esp8266/tree/master/examples/RTU#Modbus-RTU-Specific-API)
- [Callbacks](https://github.com/emelianov/modbus-esp8266/tree/master/examples/Callback/#Callback-API)
- [Modbus Security](https://github.com/emelianov/modbus-esp8266/tree/master/examples/TLS)
- [Modbus File operations](https://github.com/emelianov/modbus-esp8266/tree/master/examples/Files#File-block-API)
- [Compile-time settings](https://github.com/emelianov/modbus-esp8266/tree/master/src/ModbusSettings.h)

---

## Client work cycle diagram

![Client diagram](https://github.com/emelianov/modbus-esp8266/blob/master/resources/client.png)

---

## Server work cycle diagram

![Server diagram](https://github.com/emelianov/modbus-esp8266/blob/master/resources/server.png)

---

## How do I send a signed value (`int16_t`)?

## How do I send `float` or `uint32_t` values?

The Modbus standard defines only bit values and 16-bit values. Send all other data types as multiple 16-bit values.

---

## Why is a value not available immediately after `readCoil()`, `readHreg()`, or a similar call?

The library operates asynchronously. For example, `readHreg()` sends a read request to the Modbus server and returns immediately. The `task()` method
processes the response when it arrives and returns immediately if no data is available.

---

## Why is only the first of several consecutive read or write calls executed?

---

## Transactional callback returns _0xE4_ error

This code indicates a timeout. The suggestions below apply to persistent or frequent failures; an occasional timeout may be normal.

### ModbusRTU

This typically indicates a wiring or hardware problem.

- Check wiring.
- Check that the client and server use the same baud rate.
- Try reducing the baud rate to 9600 bps.
- Try a different power source for the Arduino device.
- Try to replace RS-485 transceiver.
- If you use Modbus simulator software on a PC, compare the result with another simulator.

### ModbusTCP

This may indicate a network problem. Use standard diagnostics such as `ping` and check the firewall settings.

---

## Is it possible to create a Modbus TCP-to-RTU pass-through bridge?

See [this code](https://github.com/emelianov/modbus-esp8266/issues/101#issuecomment-755419095) for implementation ideas. A limited implementation is
available in the [bridge examples](../examples/Bridge).

---

## Modbus Library for Arduino

### ModbusRTU, ModbusTCP and ModbusTCP Security

(c)2021 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
