// Copyright (C) 2019-2022 Alexander Emelianov. See LICENSE.txt.
#include "ModbusRTU_PICO.h"
#include <cstring>
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <utility>

static uint16_t crc16(uint8_t address, const uint8_t* pdu, size_t size) {
  uint16_t crc = 0xffff;
  for (size_t i = 0; i <= size; ++i) {
    crc ^= i == 0 ? address : pdu[i - 1];
    for (unsigned bit = 0; bit < 8; ++bit) {
      crc = static_cast<uint16_t>(crc >> 1U) ^ static_cast<uint16_t>((crc & 1U) ? 0xa001 : 0);
    }
  }
  return crc;
}

static bool validUartPin(uart_inst_t* uart, unsigned pin, bool rx) {
  // The UART assignment repeats every 16 GPIOs. RP2350 adds TX/RX on the auxiliary function.
#if PICO_RP2040
  if (pin & 2u) return false;
#endif
  return pin < NUM_BANK0_GPIOS && ((pin + 4) / 8) % 2 == uart_get_index(uart) && (pin & 1U) == static_cast<unsigned>(rx);
}

static bool validControlPin(const ModbusUartConfig& config, int pin) {
  return pin == -1 || (pin >= 0 && pin < NUM_BANK0_GPIOS && std::cmp_not_equal(pin, config.txPin) && std::cmp_not_equal(pin, config.rxPin));
}

static void configureOutput(int pin, bool level) {
  if (pin < 0) {
    return;
  }
  gpio_init(pin);
  gpio_put(pin, level);
  gpio_set_dir(pin, GPIO_OUT);
}

static bool isBroadcastWrite(uint8_t function) {
  switch (function) {
    case Modbus::FC_WRITE_COIL:
    case Modbus::FC_WRITE_REG:
    case Modbus::FC_WRITE_COILS:
    case Modbus::FC_WRITE_REGS:
    case Modbus::FC_MASKWRITE_REG:
    case Modbus::FC_WRITE_FILE_REC: return true;
    default:                        return false;
  }
}

static bool validRequestSize(const uint8_t* pdu, size_t size) {
  switch (pdu[0]) {
    case Modbus::FC_READ_COILS:
    case Modbus::FC_READ_INPUT_STAT:
    case Modbus::FC_READ_REGS:
    case Modbus::FC_READ_INPUT_REGS:
    case Modbus::FC_WRITE_COIL:
    case Modbus::FC_WRITE_REG:       return size == 5;
    case Modbus::FC_WRITE_COILS:
    case Modbus::FC_WRITE_REGS:      return size >= 6 && size == 6U + pdu[5];
    case Modbus::FC_MASKWRITE_REG:   return size == 7;
    case Modbus::FC_READWRITE_REGS:  return size >= 10 && size == 10U + pdu[9];
    case Modbus::FC_READ_FILE_REC:   return size >= 9 && (size - 2) % 7 == 0 && pdu[1] == size - 2;
    case Modbus::FC_WRITE_FILE_REC:  return size >= 11 && pdu[1] == size - 2;
    default:                         return true; // Custom function codes may be handled by onRaw.
  }
}

bool ModbusRTU::begin(const ModbusUartConfig& config) {
  if (uart_ || (config.uart != uart0 && config.uart != uart1) || !config.baud || !validUartPin(config.uart, config.txPin, false) ||
      !validUartPin(config.uart, config.rxPin, true) || !validControlPin(config, config.directionPin) ||
      !validControlPin(config, config.receiverEnablePin) || (config.receiverEnablePin >= 0 && config.receiverEnablePin == config.directionPin) ||
      (config.parity != UART_PARITY_NONE && config.parity != UART_PARITY_EVEN && config.parity != UART_PARITY_ODD)) {
    return false;
  }
  auto const baud = uart_init(config.uart, config.baud);
  if (!baud) {
    return false;
  }
  uart_ = config.uart;
  uart_set_hw_flow(uart_, false, false);
  uart_set_format(uart_, 8, config.parity == UART_PARITY_NONE ? 2 : 1, config.parity);
  uart_set_fifo_enabled(uart_, true);
  directionPin_ = config.directionPin;
  transmitLevel_ = config.transmitLevel;
  receiverEnablePin_ = config.receiverEnablePin;
  receiveLevel_ = config.receiveLevel;
  configureOutput(directionPin_, !transmitLevel_);
  configureOutput(receiverEnablePin_, receiveLevel_);
  gpio_set_function(config.txPin, UART_FUNCSEL_NUM(uart_, config.txPin));
  gpio_set_function(config.rxPin, UART_FUNCSEL_NUM(uart_, config.rxPin));
  interCharacterUs_ = config.baud > 19200 ? 750 : (16500000U + baud - 1) / baud;
  // RX timestamps mark completed characters, so two arrivals include one character on the wire.
  interCharacterUs_ += (11000000U + baud - 1) / baud;
  interFrameUs_ = config.baud > 19200 ? 1750 : (38500000U + baud - 1) / baud;
  lastActivity_ = time_us_64();
  return true;
}

void ModbusRTU::setDirection(bool transmitting) const {
  if (receiverEnablePin_ >= 0) {
    gpio_put(receiverEnablePin_, transmitting ? !receiveLevel_ : receiveLevel_);
  }
  if (directionPin_ >= 0) {
    gpio_put(directionPin_, transmitting ? transmitLevel_ : !transmitLevel_);
  }
}

void ModbusRTU::transmit(uint8_t id) {
  const auto crc = crc16(id, _frame, _len);
  const uint8_t trailer[] = { static_cast<uint8_t>(crc), static_cast<uint8_t>(crc >> 8U) };
  setDirection(true);
  uart_write_blocking(uart_, &id, 1);
  uart_write_blocking(uart_, _frame, _len);
  uart_write_blocking(uart_, trailer, sizeof(trailer));
  uart_tx_wait_blocking(uart_);
  setDirection(false);
  lastActivity_ = time_us_64();
}

uint16_t ModbusRTU::send(uint8_t id, TAddress reg, cbTransaction callback, uint8_t* output, bool waitResponse) {
  if (!uart_ || id > 247 || !_len || _len > MODBUS_MAX_FRAME || (waitResponse && !clientMode_) || (clientMode_ && address_)) {
    return 0;
  }
  if (!id && (!waitResponse || !isBroadcastWrite(_frame[0]))) {
    return 0;
  }
  if (waitResponse && !validRequestSize(_frame, _len)) {
    return 0;
  }
  if (received_ || uart_is_readable(uart_)) {
    return 0;
  }
  busy_wait_until(from_us_since_boot(lastActivity_ + interFrameUs_));
  if (uart_is_readable(uart_)) {
    return 0;
  }
  transmit(id);
  if (waitResponse && id) {
    address_ = id;
    deadline_ = lastActivity_ + 1000000;
    callback_ = std::move(callback);
    output_ = output;
    register_ = reg;
    requestSize_ = _len;
    std::memcpy(request_, _frame, _len);
  }
  return 1;
}

void ModbusRTU::finishTransaction(ResultCode result) {
  auto const callback = std::move(callback_);
  address_ = 0;
  output_ = nullptr;
  if (callback) {
    callback(result, 0, nullptr);
  }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void ModbusRTU::processFrame() {
  const size_t size = received_;
  received_ = 0;
  if (std::exchange(discard_, false) || size < 4) {
    return;
  }
  const auto id = rx_[0];
  uint8_t* pdu = rx_ + 1;
  _len = size - 3;
  if (crc16(id, pdu, _len) != (rx_[size - 2] | static_cast<uint16_t>(rx_[size - 1] << 8U))) {
    return;
  }
  if (id > 247 || (!id && (clientMode_ || !isBroadcastWrite(pdu[0])))) {
    return;
  }
  bool const addressed = address_ && (id == address_ || id == 0);
  if (!addressed && !_cbRaw) {
    return;
  }
  source_ = id;
  frame_arg_t context{ .slaveId = id, .toServer = !clientMode_ };
  auto action = _cbRaw ? _cbRaw(pdu, _len, &context) : EX_PASSTHROUGH;
  if (!addressed && action != EX_FORCE_PROCESS) {
    return;
  }
  _len = size - 3;
  if (clientMode_) {
    if (!address_ || (pdu[0] & 0x7FU) != request_[0]) {
      return;
    }
    if (action == EX_PASSTHROUGH || action == EX_FORCE_PROCESS) {
      if (pdu[0] & 0x80U) {
        action = _len == 2 ? ResultCode(pdu[1]) : EX_DATA_MISMATCH;
      } else if (isBroadcastWrite(pdu[0])) {
        size_t const echoSize = (pdu[0] == FC_WRITE_REGS || pdu[0] == FC_WRITE_COILS) ? 5 : requestSize_;
        action = _len == echoSize && !std::memcmp(request_, pdu, echoSize) ? EX_SUCCESS : EX_DATA_MISMATCH;
      } else if (_len >= 2 && pdu[1] == _len - 2) {
        masterPDU(pdu, request_, register_, output_);
        action = ResultCode(_reply);
      } else {
        {
          action = EX_DATA_MISMATCH;
        }
      }
    }
    finishTransaction(action);
  } else if (action == EX_PASSTHROUGH || action == EX_FORCE_PROCESS) {
    std::memcpy(_frame, pdu, _len); // Echo responses use the original request.
    if (validRequestSize(pdu, _len)) {
      slavePDU(pdu);
    } else {
      exceptionResponse(FunctionCode(pdu[0]), EX_ILLEGAL_VALUE);
    }
    if (id && _reply != REPLY_OFF) {
      transmit(id);
    }
  }
}

void ModbusRTU::task() {
  if (!uart_) {
    return;
  }
  auto now = time_us_64();
  if (received_ && now - lastActivity_ >= interFrameUs_) {
    processFrame();
  }
  while (uart_is_readable(uart_)) {
    now = time_us_64();
    // PL011 reports framing, parity, break and overrun in bits 8..11 of DR.
    auto const data = uart_get_hw(uart_)->dr;
    if ((data & 0xF00U) || (received_ && now - lastActivity_ > interCharacterUs_)) {
      discard_ = true;
    }
    if (received_ < sizeof(rx_)) {
      rx_[received_++] = static_cast<uint8_t>(data);
    } else {
      discard_ = true;
    }
    lastActivity_ = now;
  }
  if (clientMode_ && address_ && time_us_64() >= deadline_) {
    finishTransaction(EX_TIMEOUT);
  }
}
