/*
    Modbus Library for Arduino-pico rp2040/rp2350
    ModbusTCP for W5x00 Ethernet
    Copyright (C) 2022 Alexander Emelianov (a.m.emelianov@gmail.com)
    Copyright (C) 2026 Jimes Yang (admin@sndnvaps.com)
*/

#pragma once
#if defined(MODBUSIP_USE_DNS)
#include <Dns.h>
#endif

#if !defined(PICO_RP2040) && !defined(PICO_RP2350)
#error Unsupported architecture
#endif

#include <WiFiClientSecure.h>
#include <WiFiServerSecure.h>

#include "ModbusAPI.h"
#include "ModbusTCPTemplate.h"

#include <EthernetCompat.h> // lwIP-Ethernet compatibility layer.

using EthernetServerTLS = WiFiServerSecure;
using EthernetClientTLS = WiFiClientSecure;

class ModbusEthernetTLS : public ModbusAPI<ModbusTCPTemplate<EthernetServerTLS, EthernetClientTLS>> {
private:
  int8_t _connect(IPAddress ipAddress, uint16_t port, const char* clientCertificate = nullptr, const char* clientPrivateKey = nullptr) {
    int8_t clientIndex = getFreeClient();
    if (clientIndex < 0) return clientIndex;
    tcpClients[clientIndex] = new EthernetClientTLS();
    BIT_CLEAR(tcpServerConnection, clientIndex);

    BearSSL::X509List* clientCertList = new BearSSL::X509List(clientCertificate);
    BearSSL::PrivateKey* clientPrivKey = new BearSSL::PrivateKey(clientPrivateKey);
    tcpClients[clientIndex]->setClientRSACert(clientCertList, clientPrivKey);
    tcpClients[clientIndex]->setBufferSizes(512, 512);

    return clientIndex;
  }
#ifdef MODBUSIP_USE_DNS
  static IPAddress resolveHost(const char* host) {
    IPAddress remoteAddress;
    if (WiFi.hostByName(host, remoteAddress)) return remoteAddress;
    return IPADDR_NONE;
  }
#endif
public:
  ModbusEthernetTLS() : ModbusAPI() {
    defaultPort = MODBUSTLS_PORT;
#ifdef MODBUSIP_USE_DNS
    resolver = resolveHost;
#endif
  }

  void server(uint16_t port, const char* serverCertificate = nullptr, const char* serverPrivateKey = nullptr, const char* caCertificate = nullptr) {
    serverPort = port;
    tcpServer = new EthernetServerTLS(serverPort);
    BearSSL::X509List* serverCertList = new BearSSL::X509List(serverCertificate);
    BearSSL::PrivateKey* serverPrivKey = new BearSSL::PrivateKey(serverPrivateKey);
    tcpServer->setRSACert(serverCertList, serverPrivKey);
    if (caCertificate) {
      BearSSL::X509List* trustedCA = new BearSSL::X509List(caCertificate);
      tcpServer->setClientTrustAnchor(trustedCA);
    }
    // tcpServer->setBufferSizes(512, 512);
    tcpServer->begin();
  }

  bool connectWithKnownKey(IPAddress ipAddress, uint16_t port, const char* clientCertificate = nullptr, const char* clientPrivateKey = nullptr,
                           const char* key = nullptr) {
    if (getSlave(ipAddress) >= 0) return true;
    int8_t clientIndex = _connect(ipAddress, port, clientCertificate, clientPrivateKey);
    BearSSL::PublicKey* clientPublicKey = new BearSSL::PublicKey(key);
    tcpClients[clientIndex]->setKnownKey(clientPublicKey);
    return tcpClients[clientIndex]->connect(ipAddress, port);
  }

#ifdef MODBUSIP_USE_DNS
  bool
  connect(String host, uint16_t port, const char* clientCertificate = nullptr, const char* clientPrivateKey = nullptr, const char* caCertificate = nullptr) {
    return connect(resolveHost(host.c_str()), port, clientCertificate, clientPrivateKey, caCertificate);
  }
  bool connect(const char* host, uint16_t port, const char* clientCertificate = nullptr, const char* clientPrivateKey = nullptr,
               const char* caCertificate = nullptr) {
    return connect(resolveHost(host), port, clientCertificate, clientPrivateKey, caCertificate);
  }
#endif
  bool
  connect(IPAddress ipAddress, uint16_t port, const char* clientCertificate = nullptr, const char* clientPrivateKey = nullptr, const char* caCertificate = nullptr) {
    if (!ipAddress) return false;
    if (getSlave(ipAddress) >= 0) return false;
    int8_t clientIndex = _connect(ipAddress, port, clientCertificate, clientPrivateKey);
    if (clientIndex < 0) return false;

    if (caCertificate) {
      BearSSL::X509List* trustedCA = new BearSSL::X509List(caCertificate);
      tcpClients[clientIndex]->setTrustAnchors(trustedCA);
    } else {
      tcpClients[clientIndex]->setInsecure();
    }

    // return tcpClients[clientIndex]->connect(ipAddress, port);
    if (!tcpClients[clientIndex]->connect(ipAddress, port)) return false;
    return true;
  }
};
