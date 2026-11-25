#include "ModbusRTU_PICO.h"

int main() {
  // Bare Raspberry Pi Pico 2, UART0 on GP0/GP1. A product must select its own pins.
  static ModbusRTU bus;
  if (!bus.begin({ .uart = uart0, .txPin = 0, .rxPin = 1, .baud = 19200 }) || !bus.server(1) || !bus.addHreg(0, 0x1234, 125)) {
    return 1;
  }
  while (true) {
    bus.task();
  }
}
