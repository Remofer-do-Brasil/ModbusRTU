# Modbus Library for Arduino-Pico RP2040/RP2350

## Modbus-Pico

See the [documentation](documentation) for details about using the library.

## Features

- `ModbusRTU_PICO.h` for USB serial and TTL serial connections
- `ModbusIP_PICO.h` for Wi-Fi connections using `WiFi.h` on Arduino-Pico (Pico W and Pico 2 W)
- `ModbusTLS.h` for TLS over Wi-Fi using `WiFi.h` on Arduino-Pico (Pico W and Pico 2 W)
- `ModbusEthernet.h` for Ethernet library v2
- `ModbusLWIP_Ethernet.h` for lwIP Ethernet on Raspberry Pi Pico boards using `EthernetCompat.h`
- `ModbusLWIP_TLS_Ethernet.h` for Modbus TCP Security over lwIP Ethernet on Raspberry Pi Pico boards

* Supports RP2040 and RP2350 devices

* Operates in any combination of multiple instances of
  - [Modbus RTU server](examples/RTU)
  - [Modbus RTU client](examples/RTU)
  - [Modbus UDP/IP server](examples/UDP)
  - Modbus TCP server for [Pico W/Pico 2 W](examples/TCP-PICO) and the [Ethernet library](examples/TCP-Ethernet)
  - Modbus TCP server for the [lwIP-Ethernet library](examples/TCP-LWIP-Ethernet)
  - Modbus TCP client for [Pico W/Pico 2 W](examples/TCP-PICO) and the [Ethernet library](examples/TCP-Ethernet)
  - Modbus TCP client for the [lwIP-Ethernet library](examples/TCP-LWIP-Ethernet)
  - Modbus TCP Security server/client for the [lwIP Ethernet library](examples/TCP-LWIP-TLS-Ethernet)
  - [Modbus TCP Security server (Pico)](examples/TLS)
  - [Modbus TCP Security client (Pico W/Pico 2 W)](examples/TLS)

* Modbus functions supported:
  - 0x01 - Read Coils
  - 0x02 - Read Input Status (Read Discrete Inputs)
  - 0x03 - Read Holding Registers
  - 0x04 - Read Input Registers
  - 0x05 - Write Single Coil
  - 0x06 - Write Single Register
  - 0x0F - Write Multiple Coils
  - 0x10 - Write Multiple Registers
  - 0x14 - Read File Record
  - 0x15 - Write File Record
  - 0x16 - Mask Write Register
  - 0x17 - Read/Write multiple registers
* [Callback-driven](examples/Callback) design
* Complex real-world examples:
  - [Pico/Pico W/Pico 2 W firmware update over Modbus](examples/Files)
  - [Modbus RTU-to-TCP bridge](examples/Bridge)

## Notes

1. Register offsets are zero-based. For example, [ScadaBR](http://www.scadabr.com.br) also uses zero-based offsets, so register 100 in the library is
   register 100 in ScadaBR. [CAS Modbus Scanner](http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) uses one-based offsets,
   so register 100 in the library is register 101 in that application.
2. RS-485 transceivers based on the MAX485 operate at up to at least 115200 bps. Some XY-017/XY-485 modules operate only at up to 9600 bps.

For more information about Modbus, see:

- [Modbus (From Wikipedia, the free encyclopedia)](http://pt.wikipedia.org/wiki/Modbus)
- [MODBUS APPLICATION PROTOCOL SPECIFICATION V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)
- [MODBUS MESSAGING ON TCP/IP IMPLEMENTATION GUIDE V1.0b](http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf)
- [MODBUS over Serial Line Specification and Implementation Guide V1.02](http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf)
- [MODBUS/TCP Security Protocol Specification](https://modbus.org/docs/MB-TCP-Security-v21_2018-07-24.pdf)

## Last Changes

```diff
// 4.1.0
+ API: Raw Modbus frame processing functionality
+ ModbusRTU: Precise inter-frame interval control
+ Examples: Full Modbus RTU-to-TCP server bridge
+ Examples: One Modbus RTU device responding to multiple identifiers
+ ModbusRTU: Add direction control pin for Stream
+ STL: Limit the register count to the vector capacity of 4000 on ESP8266 and ESP32
+ Settings: Added MODBUSIP_CONNECTION_TIMEOUT (ESP32 only)
+ Settings: Set MODBUSIP_MAX_CLIENTS = 8 for ESP32
+ ModbusTCP: Make DNS-name support optional
+ ModbusRTU: Add optional separate RE/DE pin control
+ API: Drop support of Ethernet library v1
+ Examples: Teknic ClearCore ArduinoWrapper examples added
+ Examples: Modbus TCP-to-RTU example added
+ ModbusRTU: Flush extra delay optional feature
// 4.0.0
+ Support all Arduino boards
+ ModbusTLS: ESP8266 Client/Server and ESP32 Client
+ ModbusTCP: ModbusEthernet - WizNet W5x00, ENC28J60 Ethernet library support
+ 0x14 - Read File Records function
+ 0x15 - Write File Records function
+ Examples: FW update over Modbus fully functional example
+ 0x16 - Mask Write Register function
+ Test: 0x16
+ 0x17 - Read/Write Registers function
+ ModbusRTU: ESP32 SoftwareSerial support
+ Build with no STL dependency (switchable)
+ API: ModbusIP => ModbusTCP
+ API: Access control callback for individual Modbus function
+ API: Master/Slave => Client/Server according to [PRESS RELEASE](https://modbus.org/docs/Client-ServerPR-07-2020-final.docx.pdf)
+ Extensive code refactoring and small fixes
```

## Roadmap

```diff
// 4.2.0-DEV
- API: Alternative CRC calculation (reduced memory footprint)
- ModbusRTU: Static buffer allocation
- Test: Frame accuracy to specification
- Buffer/packet size limitation support
- Slave/Server: Use early returns in `slavePDU()` where possible
- Master/Client: Check frame size against header data where possible
- Master/Client: Additional response data validation
- Free global registers and callbacks when the last Modbus instance is removed
- Test: push/pull functions
- ModbusTCP: Refactor connections by DNS name using native implementations where available
// 4.3.0-DEV
- ModbusTLS: ESP32 Server
- Test: TLS ESP32 Server
- Test: TLS ESP32 Client
- Examples: TLS Certificate test Role extension and Alt-Name
- Examples: TLS Add example explanation
- ModbusTCP: ModbusAsyncTCP
- API: Extend API to allow custom Modbus commands
- Examples: Basic file operations
- Examples: Revising
```

## Contributions

https://github.com/sndnvaps/modbus-pico

Original version:

https://github.com/emelianov/modbus-esp8266

a.m.emelianov@gmail.com

https://github.com/andresarmento/modbus-esp8266

https://github.com/andresarmento/modbus-arduino

prof (at) andresarmento (dot) com

## License

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
