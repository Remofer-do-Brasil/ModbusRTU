
/*
    Modbus Library for Arduino

    Copyright (C) 2019-2022 Alexander Emelianov (a.m.emelianov@gmail.com)
        https://github.com/emelianov/modbus-esp8266
        This code is licensed under the BSD New License. See LICENSE.txt for more info.

Prefixes:
MODBUS_     Global library settings
MODBUSIP_   Settings for TCP and TLS both
MODBUSTCP_  Settings for TCP
MODBUSTLS_  Settings for TLS
MODBUSRTU_  Settings for RTU
MODBUSAPI_  Settings for API
*/
#pragma once

/*
#define MODBUS_GLOBAL_REGS
If defined Modbus registers will be shared across all Modbus* instances.
If not defined each Modbus object will have own registers set.
*/
#define MODBUS_GLOBAL_REGS
// #define MODBUS_FREE_REGS

/*
#define ARDUINO_SAM_DUE_STL
Use the STL with Arduino Due. It works with the Arduino IDE but not with PlatformIO.
Also note the STL workaround in Modbus.cpp.
*/
#if defined(ARDUINO_SAM_DUE)
// #define ARDUINO_SAM_DUE_STL
#endif

/*
#define MODBUS_USE_STL
If defined, the C++ STL will be used.
*/
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_SAM_DUE_STL) || defined(PICO_RP2040) || defined(PICO_RP2350)
#define MODBUS_USE_STL
#endif

/*
#define MODBUS_MAX_REGS     32
If defined, the register count will be limited.
*/
// Add a limit for specific STL implementations.
#if defined(MODBUS_USE_STL) && (defined(ESP8266) || defined(ESP32) || defined(PICO_RP2040) || defined(PICO_RP2350))
#undef MODBUS_MAX_REGS
#define MODBUS_MAX_REGS 32
#endif

#define MODBUS_ADD_REG
// #define MODBUS_STRICT_REG
#define MODBUS_MAX_FRAME 128
// #define MODBUS_STATIC_FRAME
#define MODBUS_MAX_WORDS 0x007D
#define MODBUS_MAX_BITS  0x07D0
#define MODBUS_FILES
#define MODBUSTCP_PORT    502
#define MODBUSTLS_PORT    802
#define MODBUSIP_MAXFRAME 200

/*
ModbusTCP and ModbusTLS timeouts
#define MODBUSIP_TIMEOUT 1000
Outgoing request timeout
#define MODBUSIP_CONNECT_TIMEOUT 1000
ESP32 only. Outgoing connection-attempt timeout.
*/
#define MODBUSIP_TIMEOUT 1000
// #define MODBUSIP_CONNECT_TIMEOUT 1000

#define MODBUSIP_UNIT             1
#define MODBUSIP_MAX_TRANSACTIONS 16
#if defined(ESP32)
#define MODBUSIP_MAX_CLIENTS 8
#else
#define MODBUSIP_MAX_CLIENTS 4
#endif
#define MODBUSIP_UNIQUE_CLIENTS
#define MODBUSIP_MAX_READMS 100
#define MODBUSIP_FULL

#define MODBUSIP_DEBUG 1
/*
Allows DNS names to be used as targets.
Otherwise, only IP addresses may be used.
#define MODBUS_IP_USE_DNS
*/
// #define MODBUS_IP_USE_DNS

// #define MODBUSRTU_DEBUG
#define MODBUSRTU_BROADCAST  0
#define MB_RESERVE           248
#define MB_SERIAL_BUFFER     128
#define MODBUSRTU_TIMEOUT    1000
#define MODBUSRTU_MAX_READMS 100
/*
#define MODBUSRTU_REDE
Enable separate pins for RE and DE.
*/
// #define MODBUSRTU_REDE

// Define for internal use. Do not change.
#define MODBUSRTU_TIMEOUT_US  1000UL * MODBUSRTU_TIMEOUT
#define MODBUSRTU_MAX_READ_US 1000UL * MODBUSRTU_MAX_READMS

/*
#define MODBUSRTU_FLUSH_DELAY 1
Set an extra delay after flushing the serial buffer and before changing the RE/DE pin state.
The value is specified in characters. A value of 1 adds enough delay to send one character at the current port baud rate.
*/
// #define MODBUSRTU_FLUSH_DELAY 1

#define MODBUSRTU_REDE_SWITCH_US 1000

#define MODBUSAPI_LEGACY
#define MODBUSAPI_OPTIONAL

// Workaround for RP2040 flush() bug
#if defined(ARDUINO_ARCH_RP2040) || defined(PICO_RP2040) || defined(PICO_RP2350)
#define MODBUSRTU_FLUSH_DELAY 1
#endif

// Limit resource usage on entry-level boards.
#if defined(ARDUINO_UNO) || defined(ARDUINO_LEONARDO)
#undef MODBUS_MAX_REGS
#undef MODBUSIP_MAX_TRANSACTIONS
#undef MODBUS_MAX_WORDS
#undef MODBUS_MAX_BITS
#define MODBUS_MAX_REGS           32
#define MODBUSIP_MAX_TRANSACTIONS 4
#define MODBUS_MAX_WORDS          0x0020
#define MODBUS_MAX_BITS           0x0200
#endif
