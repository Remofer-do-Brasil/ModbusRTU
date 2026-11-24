/*
  Modbus Library for Arduino Example - Modbus RTU Firmware Update - ESP8266/ESP32
  Firmware-update target node.
  Receives and flashes firmware sent by the source node.

  (c)2021 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  (c)2026 Jimes Yang (admin@sndnvaps.com)
  https://github.com/sndnvaps/modbus-pico
*/
#include <ModbusRTU_PICO.h>

#include <LittleFS.h> // Required by Updater.h.
#include <Updater.h>

#if defined(PICO_RP2040) || defined(PICO_RP2350)

#define PICO_FS_SIZE 1048576 // 1024 * 1024; set the file-system size to 1 MB.

#include <SoftwareSerial.h> // SoftwareSerial for Raspberry Pi Pico W/Pico 2 W.

// SoftwareSerial softwareSerial(26, 27, false);

// Constructor arguments: receivePin, transmitPin, inverseLogic.
// Connect RX to GP4 (GPIO4) and TX to GP5 (GPIO5).
SoftwareSerial softwareSerial(4, 5);
#endif

ModbusRTU rtu;

#define UPDATE_ENABLE 301
#define UPDATE_FILE   301
#define SLAVE_ID      1
uint32_t written = 0;
uint16_t handleUpdateEnable(TRegister* reg, uint16_t value) {
  uint32_t sketchSpace;
  if (rtu.Reg(reg->address)) {
    if (COIL_BOOL(value)) return BIT_VAL(true);
    Serial.println("!");
    Serial.print("Written: ");
    Serial.println(written * 2);
    if (Update.end(true)) { // Pass true to use the current progress as the final size.
      Serial.println("Update Success. \nRebooting...");
    }
    return BIT_VAL(false);
  }
  /*
    #ifdef ESP8266
    sketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
   #else
    sketchSpace = ESP32_SKETCH_SIZE;
   #endif
   */
#if defined(PICO_RP2040) || defined(PICO_RP2350)
  sketchSpace = PICO_FS_SIZE;
#endif

  Serial.printf("Starting update. FW maximum size: %ld\n", sketchSpace);
  if (!Update.begin(sketchSpace)) {
    Update.printError(Serial);
    return BIT_VAL(false);
  }
  written = 0;
  return BIT_VAL(true);
}

// Expected arguments are:
// functionCode = FC_WRITE_FILE_REC
// fileNumber = UPDATE_FILE
// recordNumber is ignored
// recordLength = data size, in words
// frame = data buffer to write
Modbus::ResultCode handleFile(Modbus::FunctionCode functionCode, uint16_t fileNumber, uint16_t recordNumber, uint16_t recordLength, uint8_t* frame) {
  (void)recordNumber;
  if (functionCode != Modbus::FC_WRITE_FILE_REC) {
    Serial.println("X:func");
    return Modbus::EX_ILLEGAL_FUNCTION;
  }
  if (!rtu.Reg(COIL(UPDATE_ENABLE))) {
    Serial.println("X:idle");
    return Modbus::EX_ILLEGAL_VALUE;
  }
  if (fileNumber != UPDATE_FILE) {
    Serial.println("X:file");
    return Modbus::EX_ILLEGAL_VALUE;
  }
  Serial.print(".");
  if (Update.write(frame, recordLength * 2) != recordLength * 2) {
    Update.printError(Serial);
  }
  written += recordLength;
  return Modbus::EX_SUCCESS;
}

void setup() {
  Serial.begin(115200);
#if defined(PICO_RP2040) || defined(PICO_RP2350)
  softwareSerial.begin(115200, SERIAL_8N1);
  rtu.begin(&softwareSerial);
#endif
  rtu.server(SLAVE_ID);
  rtu.onFile(handleFile);
  rtu.addReg(COIL(UPDATE_ENABLE));
  rtu.onSet(COIL(UPDATE_ENABLE), handleUpdateEnable);
}

void loop() {
  rtu.task();
  yield();
}
