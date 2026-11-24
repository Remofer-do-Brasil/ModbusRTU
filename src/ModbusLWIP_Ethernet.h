/*
    Modbus Library for Arduino
    ModbusTCP for W5x00 Ethernet
    Copyright (C) 2022 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#if defined(MODBUSIP_USE_DNS)
#include <Dns.h>
#endif

#if !defined(PICO_RP2040) && !defined(PICO_RP2350)
#error Unsupported architecture
#endif

#include "ModbusAPI.h"

#include "ModbusTCPTemplate.h"

#include <EthernetCompat.h> // lwIP-Ethernet compatibility layer.
#include <WiFi.h>

class ModbusEthernet : public ModbusAPI<ModbusTCPTemplate<EthernetServer, EthernetClient>> {
#ifdef MODBUSIP_USE_DNS
private:
  static IPAddress resolveHost(const char* host) {
    DNSClient dns;
    IPAddress ipAddress;

    dns.begin(Ethernet.dnsServerIP());
    if (dns.getHostByName(host, ipAddress) == 1) return ipAddress;
    else return IPADDR_NONE;
  }

public:
  ModbusEthernet() : ModbusAPI() { resolver = resolveHost; }
#endif
};
