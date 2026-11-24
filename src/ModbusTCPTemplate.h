/*
    Modbus Library for Arduino
    ModbusTCP general implementation
    Copyright (C) 2014 Andre Sarmento Barbosa
                  2017-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
    Modified for UDP Support
*/

#pragma once
#include "Modbus.h"

enum ModbusTransportMode { MODE_TCP, MODE_UDP };
using mode_type = ModbusTransportMode; // Deprecated type name retained for source compatibility.

#define BIT_SET(value, bitIndex)   ((value) |= (1ULL << (bitIndex)))
#define BIT_CLEAR(value, bitIndex) ((value) &= ~(1ULL << (bitIndex)))
#define BIT_CHECK(value, bitIndex) (!!((value) & (1ULL << (bitIndex))))
#ifndef IPADDR_NONE
#define IPADDR_NONE ((uint32_t)0xffffffffUL)
#endif

// Callback function Type
#ifdef MODBUS_USE_STL
typedef std::function<bool(IPAddress)> cbModbusConnect;
typedef std::function<IPAddress(const char*)> cbModbusResolver;
#else
typedef bool (*cbModbusConnect)(IPAddress ipAddress);
typedef IPAddress (*cbModbusResolver)(const char*);
#endif

struct TTransaction {
  uint16_t transactionId;
  uint32_t timestamp;
  cbTransaction callbackFunction = nullptr;
  uint8_t* _frame = nullptr;
  uint8_t* data = nullptr;
  TAddress startRegister;
  Modbus::ResultCode forcedEvent = Modbus::EX_SUCCESS;
  bool operator==(const TTransaction& obj) const { return transactionId == obj.transactionId; }
};

template <class SERVER, class CLIENT> class ModbusTCPTemplate : public Modbus {
protected:
  EthernetUDP udp;
  bool udpStarted = false;

  ModbusTransportMode transportMode = MODE_TCP;

  union MbapHeader {
    struct {
      uint16_t transactionId;
      uint16_t protocolId;
      uint16_t length;
      uint8_t unitId;
    };
    uint8_t raw[7];
  };
  cbModbusConnect connectCallback = nullptr;
  cbModbusConnect disconnectCallback = nullptr;
  SERVER* tcpServer = nullptr;
  CLIENT* tcpClients[MODBUSIP_MAX_CLIENTS];
#if MODBUSIP_MAX_CLIENTS <= 8
  uint8_t tcpServerConnection = 0;
#elif MODBUSIP_MAX_CLIENTS <= 16
  uint16_t tcpServerConnection = 0;
#else
  uint32_t tcpServerConnection = 0;
#endif
#ifdef MODBUS_USE_STL
  std::vector<TTransaction> transactions;
#else
  DArray<TTransaction, 2, 2> transactions;
#endif
  int16_t transactionId = 1;
  int8_t currentClientIndex = -1;
  bool autoConnectMode = false;
  uint16_t serverPort = 0;
  uint16_t defaultPort = MODBUSTCP_PORT;
  cbModbusResolver resolver = nullptr;
  TTransaction* searchTransaction(uint16_t transactionIdentifier);
  void cleanupConnections();
  void cleanupTransactions();

  int8_t getFreeClient();
  int8_t getSlave(IPAddress ipAddress);
  int8_t getMaster(IPAddress ipAddress);

public:
  // MODE = MODE_TCP
  // MODE = MODE_UDP
  void setMode(ModbusTransportMode mode = MODE_TCP);

  uint16_t send(String host, TAddress startRegister, cbTransaction callbackFunction, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr,
                bool waitResponse = true);
  uint16_t send(const char* host, TAddress startRegister, cbTransaction callbackFunction, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr,
                bool waitResponse = true);
  uint16_t send(IPAddress ipAddress, TAddress startRegister, cbTransaction callbackFunction, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr,
                bool waitResponse = true);

public:
  ModbusTCPTemplate();
  ~ModbusTCPTemplate();
  bool isTransaction(uint16_t transactionIdentifier);
#if defined(MODBUSIP_USE_DNS)
  bool isConnected(String host);
  bool isConnected(const char* host);
  bool connect(String host, uint16_t port = 0);
  bool connect(const char* host, uint16_t port = 0);
  bool disconnect(String host);
  bool disconnect(const char* host);
#endif
  bool isConnected(IPAddress ipAddress);
  bool connect(IPAddress ipAddress, uint16_t port = 0);
  bool disconnect(IPAddress ipAddress);
  void server(uint16_t port = 0);
  inline void slave(uint16_t port = 0) { server(port); }
  inline void master() { client(); }
  inline void begin() { server(); };
  void client();
  void task();
  void onConnect(cbModbusConnect callbackFunction = nullptr);
  void onDisconnect(cbModbusConnect callbackFunction = nullptr);
  uint32_t eventSource() override;
  void autoConnect(bool enabled = true);
  void dropTransactions();
  uint16_t setTransactionId(uint16_t transactionIdentifier);
#if defined(MODBUS_USE_STL)
  static IPAddress defaultResolver(const char* host) {
    (void)host;
    return IPADDR_NONE;
  }
#else
  static IPAddress defaultResolver(const char* host) {
    (void)host;
    return IPADDR_NONE;
  }
#endif
};

template <class SERVER, class CLIENT> ModbusTCPTemplate<SERVER, CLIENT>::ModbusTCPTemplate() {
  for (uint8_t index = 0; index < MODBUSIP_MAX_CLIENTS; index++) tcpClients[index] = nullptr;
  resolver = defaultResolver;
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::client() {
  if (transportMode == MODE_UDP) {
    // Start the UDP listener in UDP mode.
    if (!udpStarted) {
      udp.begin(0);
      udpStarted = true;
    }
  }
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::setMode(ModbusTransportMode mode) { transportMode = mode; }

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::server(uint16_t port) {
  if (port) serverPort = port;
  else serverPort = defaultPort;

  if (transportMode == MODE_UDP) {
    // Start the UDP listener in UDP mode.
    if (!udpStarted) {
      udp.begin(serverPort);
      udpStarted = true;
    }
  } else {
    // Start the TCP server in TCP mode.
    tcpServer = new SERVER(serverPort);
    tcpServer->begin();
  }
}

#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::connect(String host, uint16_t port) {
  return connect(resolver(host.c_str()), port);
}

template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::connect(const char* host, uint16_t port) {
  return connect(resolver(host), port);
}
#endif

template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::connect(IPAddress ipAddress, uint16_t port) {
  if (transportMode == MODE_UDP) return true;
  if (!ipAddress) return false;
  if (getSlave(ipAddress) != -1) return true;
  int8_t clientIndex = getFreeClient();
  if (clientIndex == -1) return false;
  tcpClients[clientIndex] = new CLIENT();
  BIT_CLEAR(tcpServerConnection, clientIndex);
#if defined(ESP32) && defined(MODBUSIP_CONNECT_TIMEOUT)
  if (!tcpClients[clientIndex]->connect(ipAddress, port ? port : defaultPort, MODBUSIP_CONNECT_TIMEOUT)) {
#else
  if (!tcpClients[clientIndex]->connect(ipAddress, port ? port : defaultPort)) {
#endif
    delete (tcpClients[clientIndex]);
    tcpClients[clientIndex] = nullptr;
    return false;
  }
  return true;
}

template <class SERVER, class CLIENT> uint32_t ModbusTCPTemplate<SERVER, CLIENT>::eventSource() {
  if (currentClientIndex >= 0 && currentClientIndex < MODBUSIP_MAX_CLIENTS && tcpClients[currentClientIndex])
#if !defined(ethernet_h)
    return (uint32_t)tcpClients[currentClientIndex]->remoteIP();
#else
    return 1;
#endif
  return (uint32_t)INADDR_NONE;
}

template <class SERVER, class CLIENT> TTransaction* ModbusTCPTemplate<SERVER, CLIENT>::searchTransaction(uint16_t transactionIdentifier) {
#define MODBUSIP_COMPARE_TRANS [transactionIdentifier](TTransaction& transaction) { return transaction.transactionId == transactionIdentifier; }
#if defined(MODBUS_USE_STL)
  std::vector<TTransaction>::iterator iterator = std::find_if(transactions.begin(), transactions.end(), MODBUSIP_COMPARE_TRANS);
  if (iterator != transactions.end()) return &*iterator;
  return nullptr;
#else
  return transactions.entry(transactions.find(MODBUSIP_COMPARE_TRANS));
#endif
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::task() {
  MbapHeader mbapHeader;
  uint32_t taskStart = millis();

  // Process UDP traffic.
  if (transportMode == MODE_UDP && udpStarted) {
    int len = udp.parsePacket();
    if (len >= 7) {
      IPAddress remoteIpAddress = udp.remoteIP();
      uint16_t remotePort = udp.remotePort();

      // Read the MBAP header.
      udp.read(mbapHeader.raw, 7);
      _len = __swap_16(mbapHeader.length) - 1;

      if (_len > MODBUSIP_MAXFRAME || _len <= 0) {
        udp.flush();
        return;
      }

      // Read the Modbus data.
      free(_frame);
      _frame = (uint8_t*)malloc(_len);
      if (!_frame) {
        udp.flush();
        return;
      }
      udp.read(_frame, _len);

      _reply = EX_PASSTHROUGH;
      // Process the server request.
      slavePDU(_frame);

      // Send the UDP response.
      if (_reply != REPLY_OFF) {
        mbapHeader.length = __swap_16(_len + 1);
        size_t sendLength = sizeof(mbapHeader.raw) + _len;
        uint8_t* sendBuffer = (uint8_t*)malloc(sendLength);
        if (sendBuffer) {
          memcpy(sendBuffer, mbapHeader.raw, sizeof(mbapHeader.raw));
          memcpy(sendBuffer + sizeof(mbapHeader.raw), _frame, _len);
          udp.beginPacket(remoteIpAddress, remotePort);
          udp.write(sendBuffer, sendLength);
          udp.endPacket();
          free(sendBuffer);
        }
      }

      free(_frame);
      _frame = nullptr;
      _len = 0;
    }
  }

  // Process TCP traffic.
  cleanupConnections();
  if (tcpServer && transportMode == MODE_TCP) {
    CLIENT acceptedClient;
    while (millis() - taskStart < MODBUSIP_MAX_READMS && (acceptedClient = tcpServer->accept())) {
      CLIENT* currentClient = new CLIENT(acceptedClient);
      if (!currentClient || !currentClient->connected()) continue;
      if (connectCallback == nullptr || connectCallback(currentClient->remoteIP())) {
#if defined(MODBUSIP_UNIQUE_CLIENTS)
        currentClientIndex = getMaster(currentClient->remoteIP());
        if (currentClientIndex != -1) {
          tcpClients[currentClientIndex]->flush();
          delete tcpClients[currentClientIndex];
          tcpClients[currentClientIndex] = nullptr;
        }
#endif
        currentClientIndex = getFreeClient();
        if (currentClientIndex > -1) {
          tcpClients[currentClientIndex] = currentClient;
          BIT_SET(tcpServerConnection, currentClientIndex);
          continue;
        }
      }
      delete currentClient;
    }
  }

  for (currentClientIndex = 0; currentClientIndex < MODBUSIP_MAX_CLIENTS; currentClientIndex++) {
    if (!tcpClients[currentClientIndex]) continue;
    if (!tcpClients[currentClientIndex]->connected()) continue;
    if (transportMode != MODE_TCP) break;

    while (millis() - taskStart < MODBUSIP_MAX_READMS && (size_t)tcpClients[currentClientIndex]->available() > sizeof(mbapHeader)) {
      tcpClients[currentClientIndex]->readBytes(mbapHeader.raw, sizeof(mbapHeader.raw));

      if (__swap_16(mbapHeader.protocolId) != 0) {
        while (tcpClients[currentClientIndex]->available()) tcpClients[currentClientIndex]->read();
        continue;
      }
      _len = __swap_16(mbapHeader.length);
      _len--;
      if (_len > MODBUSIP_MAXFRAME) {
        exceptionResponse((FunctionCode)tcpClients[currentClientIndex]->read(), EX_SLAVE_FAILURE);
        _len--;
        for (uint8_t index = 0; tcpClients[currentClientIndex]->available() && index < _len; index++) tcpClients[currentClientIndex]->read();
      } else {
        free(_frame);
        _frame = (uint8_t*)malloc(_len);
        if (!_frame) {
          exceptionResponse((FunctionCode)tcpClients[currentClientIndex]->read(), EX_SLAVE_FAILURE);
          for (uint8_t index = 0; tcpClients[currentClientIndex]->available() && index < _len; index++) tcpClients[currentClientIndex]->read();
        } else {
          if (tcpClients[currentClientIndex]->readBytes(_frame, _len) < _len) {
            exceptionResponse((FunctionCode)_frame[0], EX_ILLEGAL_VALUE);
          } else {
            _reply = EX_PASSTHROUGH;
            if (_cbRaw) {
              frame_arg_t frameContext = { mbapHeader.unitId, tcpClients[currentClientIndex]->remoteIP(), __swap_16(mbapHeader.transactionId),
                                           BIT_CHECK(tcpServerConnection, currentClientIndex) };
              _reply = _cbRaw(_frame, _len, &frameContext);
            }
            if (BIT_CHECK(tcpServerConnection, currentClientIndex)) {
              if (_reply == EX_PASSTHROUGH) slavePDU(_frame);
              else _reply = REPLY_OFF;
            } else {
              TTransaction* transaction = searchTransaction(__swap_16(mbapHeader.transactionId));
              if (transaction) {
                if ((_frame[0] & 0x7F) == transaction->_frame[0]) {
                  if (_reply == EX_PASSTHROUGH) masterPDU(_frame, transaction->_frame, transaction->startRegister, transaction->data);
                } else {
                  _reply = EX_UNEXPECTED_RESPONSE;
                }
                if (transaction->callbackFunction) {
                  transaction->callbackFunction((ResultCode)_reply, transaction->transactionId, nullptr);
                }
                free(transaction->_frame);
#if defined(MODBUS_USE_STL)
                std::vector<TTransaction>::iterator iterator = std::find(transactions.begin(), transactions.end(), *transaction);
                if (iterator != transactions.end()) transactions.erase(iterator);
#else
                size_t transactionIndex = transactions.find([transaction](TTransaction& entry) { return *transaction == entry; });
                transactions.remove(transactionIndex);
#endif
              }
            }
          }
        }
      }
      if (!BIT_CHECK(tcpServerConnection, currentClientIndex)) _reply = REPLY_OFF;
      if (_reply != REPLY_OFF) {
        mbapHeader.length = __swap_16(_len + 1);
        size_t sendLength = (uint16_t)_len + sizeof(mbapHeader.raw);
        uint8_t* sendBuffer = (uint8_t*)malloc(sendLength);
        if (sendBuffer) {
          memcpy(sendBuffer, mbapHeader.raw, sizeof(mbapHeader.raw));
          memcpy(sendBuffer + sizeof(mbapHeader.raw), _frame, _len);
          tcpClients[currentClientIndex]->write(sendBuffer, sendLength);
          free(sendBuffer);
        }
      }
      if (_frame) {
        free(_frame);
        _frame = nullptr;
      }
      _len = 0;
    }
  }
  currentClientIndex = -1;
  cleanupTransactions();
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(IPAddress ipAddress, TAddress startRegister, cbTransaction callbackFunction, uint8_t unit,
                                                 uint8_t* data, bool waitResponse) {
  MbapHeader mbapHeader;
  uint16_t result = 0;

#if defined(MODBUSIP_MAX_TRANSACTIONS)
  if (transactions.size() >= MODBUSIP_MAX_TRANSACTIONS) goto cleanup;
#endif
  if (!ipAddress) return 0;

  // Send in UDP mode.
  if (transportMode == MODE_UDP) {
    mbapHeader.transactionId = __swap_16(transactionId);
    mbapHeader.protocolId = __swap_16(0);
    mbapHeader.length = __swap_16(_len + 1);
    mbapHeader.unitId = unit;

    size_t sendLength = sizeof(mbapHeader.raw) + _len;
    uint8_t* sendBuffer = (uint8_t*)malloc(sendLength);
    if (!sendBuffer) goto cleanup;
    memcpy(sendBuffer, mbapHeader.raw, sizeof(mbapHeader.raw));
    memcpy(sendBuffer + sizeof(mbapHeader.raw), _frame, _len);

    udp.beginPacket(ipAddress, serverPort);
    udp.write(sendBuffer, sendLength);
    udp.endPacket();
    free(sendBuffer);

    result = transactionId;
    transactionId++;
    if (!transactionId) transactionId = 1;
    goto cleanup;
  }

  // Send in TCP mode.
  int8_t clientIndex;
  if (tcpServer) {
    clientIndex = getMaster(ipAddress);
  } else {
    clientIndex = getSlave(ipAddress);
  }
  if (clientIndex == -1 || !tcpClients[clientIndex]->connected()) {
    if (!autoConnectMode) goto cleanup;
    if (!connect(ipAddress)) goto cleanup;
  }
  mbapHeader.transactionId = __swap_16(transactionId);
  mbapHeader.protocolId = __swap_16(0);
  mbapHeader.length = __swap_16(_len + 1);
  mbapHeader.unitId = unit;
  bool writeResult;
  {
    size_t sendLength = _len + sizeof(mbapHeader.raw);
    uint8_t* sendBuffer = (uint8_t*)malloc(sendLength);
    if (!sendBuffer) goto cleanup;
    memcpy(sendBuffer, mbapHeader.raw, sizeof(mbapHeader.raw));
    memcpy(sendBuffer + sizeof(mbapHeader.raw), _frame, _len);
    writeResult = (tcpClients[clientIndex]->write(sendBuffer, sendLength) == sendLength);
    free(sendBuffer);
  }
  if (!writeResult) goto cleanup;
  if (waitResponse) {
    TTransaction transactionEntry;
    transactionEntry.transactionId = transactionId;
    transactionEntry.timestamp = millis();
    transactionEntry.callbackFunction = callbackFunction;
    transactionEntry.data = data;
    transactionEntry._frame = _frame;
    transactionEntry.startRegister = startRegister;
    transactions.push_back(transactionEntry);
    _frame = nullptr;
  }
  result = transactionId;
  transactionId++;
  if (!transactionId) transactionId = 1;

cleanup:
  free(_frame);
  _frame = nullptr;
  _len = 0;
  return result;
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(String host, TAddress startRegister, cbTransaction callbackFunction, uint8_t unit, uint8_t* data,
                                                 bool waitResponse) {
  return send(resolver(host.c_str()), startRegister, callbackFunction, unit, data, waitResponse);
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(const char* host, TAddress startRegister, cbTransaction callbackFunction, uint8_t unit,
                                                 uint8_t* data, bool waitResponse) {
  return send(resolver(host), startRegister, callbackFunction, unit, data, waitResponse);
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::onConnect(cbModbusConnect callbackFunction) {
  connectCallback = callbackFunction;
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::onDisconnect(cbModbusConnect callbackFunction) {
  disconnectCallback = callbackFunction;
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::cleanupConnections() {
  for (uint8_t index = 0; index < MODBUSIP_MAX_CLIENTS; index++) {
    if (tcpClients[index] && !tcpClients[index]->connected()) {
      delete tcpClients[index];
      tcpClients[index] = nullptr;
      if (disconnectCallback && cbEnabled) disconnectCallback(IPADDR_NONE);
    }
  }
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::cleanupTransactions() {
#if defined(MODBUS_USE_STL)
  for (auto iterator = transactions.begin(); iterator != transactions.end();) {
    if (millis() - iterator->timestamp > MODBUSIP_TIMEOUT || iterator->forcedEvent != Modbus::EX_SUCCESS) {
      Modbus::ResultCode res = (iterator->forcedEvent != Modbus::EX_SUCCESS) ? iterator->forcedEvent : Modbus::EX_TIMEOUT;
      if (iterator->callbackFunction) iterator->callbackFunction(res, iterator->transactionId, nullptr);
      free(iterator->_frame);
      iterator = transactions.erase(iterator);
    } else iterator++;
  }
#else
  size_t index = 0;
  while (index < transactions.size()) {
    TTransaction transaction = transactions[index];
    if (millis() - transaction.timestamp > MODBUSIP_TIMEOUT || transaction.forcedEvent != Modbus::EX_SUCCESS) {
      Modbus::ResultCode res = (transaction.forcedEvent != Modbus::EX_SUCCESS) ? transaction.forcedEvent : Modbus::EX_TIMEOUT;
      if (transaction.callbackFunction) transaction.callbackFunction(res, transaction.transactionId, nullptr);
      free(transaction._frame);
      transactions.remove(index);
    } else index++;
  }
#endif
}

template <class SERVER, class CLIENT> int8_t ModbusTCPTemplate<SERVER, CLIENT>::getFreeClient() {
  for (uint8_t index = 0; index < MODBUSIP_MAX_CLIENTS; index++)
    if (!tcpClients[index]) return index;
  return -1;
}

template <class SERVER, class CLIENT> int8_t ModbusTCPTemplate<SERVER, CLIENT>::getSlave(IPAddress ipAddress) {
  for (uint8_t index = 0; index < MODBUSIP_MAX_CLIENTS; index++)
    if (tcpClients[index] && tcpClients[index]->connected() && tcpClients[index]->remoteIP() == ipAddress && !BIT_CHECK(tcpServerConnection, index))
      return index;
  return -1;
}

template <class SERVER, class CLIENT> int8_t ModbusTCPTemplate<SERVER, CLIENT>::getMaster(IPAddress ipAddress) {
  for (uint8_t index = 0; index < MODBUSIP_MAX_CLIENTS; index++)
    if (tcpClients[index] && tcpClients[index]->connected() && tcpClients[index]->remoteIP() == ipAddress && BIT_CHECK(tcpServerConnection, index))
      return index;
  return -1;
}

template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::isTransaction(uint16_t transactionIdentifier) {
  return searchTransaction(transactionIdentifier) != nullptr;
}
#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(String host) { return isConnected(resolver(host.c_str())); }

template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(const char* host) { return isConnected(resolver(host)); }
#endif

template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(IPAddress ipAddress) {
  if (transportMode == MODE_UDP) return true;
  if (!ipAddress) return false;
  int8_t clientIndex = getSlave(ipAddress);
  return clientIndex != -1 && tcpClients[clientIndex]->connected();
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::autoConnect(bool enabled) { autoConnectMode = enabled; }

#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(String host) { return disconnect(resolver(host.c_str())); }

template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(const char* host) { return disconnect(resolver(host)); }
#endif

template <class SERVER, class CLIENT> bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(IPAddress ipAddress) {
  if (transportMode == MODE_UDP) return true;
  if (!ipAddress) return false;
  int8_t clientIndex = getSlave(ipAddress);
  if (clientIndex != -1) {
    tcpClients[clientIndex]->stop();
    delete tcpClients[clientIndex];
    tcpClients[clientIndex] = nullptr;
    return true;
  }
  return false;
}

template <class SERVER, class CLIENT> void ModbusTCPTemplate<SERVER, CLIENT>::dropTransactions() {
#if defined(MODBUS_USE_STL)
  for (auto& transaction : transactions) transaction.forcedEvent = EX_CANCEL;
#else
  for (size_t index = 0; index < transactions.size(); index++) transactions.entry(index)->forcedEvent = EX_CANCEL;
#endif
}

template <class SERVER, class CLIENT> ModbusTCPTemplate<SERVER, CLIENT>::~ModbusTCPTemplate() {
  free(_frame);
  dropTransactions();
  cleanupConnections();
  cleanupTransactions();
  delete tcpServer;
  tcpServer = nullptr;
  for (uint8_t index = 0; index < MODBUSIP_MAX_CLIENTS; index++) {
    delete tcpClients[index];
    tcpClients[index] = nullptr;
  }
  if (udpStarted) {
    udp.stop();
  }
}

template <class SERVER, class CLIENT> uint16_t ModbusTCPTemplate<SERVER, CLIENT>::setTransactionId(uint16_t transaction) {
  transactionId = transaction;
  if (!transactionId) transactionId = 1;
  return transactionId;
}
