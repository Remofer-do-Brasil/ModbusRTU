/*
  Modbus Library for Arduino Example - Modbus RTU Firmware Update - ESP8266/ESP32
  Firmware-update source node.
  Hosts a simple web server that accepts a firmware file and sends it to the target node.

  (c)2021 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  (c)2026 Jimes Yang (admin@sndnvaps.com)
  Adapted to support Raspberry Pi Pico W/Pico 2 W.
*/
#include <ModbusRTU_PICO.h>
#include <Updater.h>
#include <WebServer.h>
#include <WiFi.h>

#if defined(PICO_RP2040) || defined(PICO_RP2350)
#include <SoftwareSerial.h> // SoftwareSerial for Raspberry Pi Pico W/Pico 2 W.

// SoftwareSerial softwareSerial(26, 27, false);

// Constructor arguments: receivePin, transmitPin, inverseLogic.
// Connect RX to GP4 (GPIO4) and TX to GP5 (GPIO5).
SoftwareSerial softwareSerial(4, 5);
#endif

ModbusRTU rtu;
WebServer web(80);

#define UPDATE_ENABLE 301
#define UPDATE_FILE   301
#define SLAVE_ID      1
#define BLOCK_SIZE    64

uint32_t written = 0;
bool updating = false;
Modbus::ResultCode result = Modbus::EX_GENERAL_FAILURE;
bool transactionCallback(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Modbus transaction callback.
  (void)transactionId;
  (void)data;
  if (event != Modbus::EX_SUCCESS)                 // If the transaction failed,
    Serial.printf("Modbus result: %02X\n", event); // display the Modbus error code.
  result = event;
  return true;
}

void handlePage() {
  String output = F(R"EOF(
<html><body>
 <form method='POST' action='/update' enctype='multipart/form-data'>
  Update firmware:<br>
  <input type='file' name='update'> <input type='submit' value='Update firmware'>
 </form>
</body></html>
)EOF");
  web.sendHeader("Connection", "close");
  web.sendHeader("Cache-Control", "no-store, must-revalidate");
  web.sendHeader("Access-Control-Allow-Origin", "*");
  web.send(200, "text/html; charset=utf-8", output);
}

void updateHandle() {
  web.sendHeader("Connection", "close");
  web.sendHeader("Refresh", "10; url=/");
  web.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
}

void updateUploadHandle() {
  uint8_t* data = nullptr;
  uint16_t remaining;
  HTTPUpload& upload = web.upload();
  switch (upload.status) {
    case UPLOAD_FILE_START:
      result = Modbus::EX_GENERAL_FAILURE;
      rtu.writeCoil(SLAVE_ID, UPDATE_ENABLE, true, transactionCallback);
      while (rtu.server()) {
        rtu.task();
        yield();
      }
      updating = (result == Modbus::EX_SUCCESS);
      written = 0;
      Serial.print("O");
      break;
    case UPLOAD_FILE_WRITE:
      if (!updating) break;
      Serial.print("o");
      data = upload.buf;
      remaining = upload.currentSize / 2;
      while (remaining) {
        uint16_t amount = (remaining > BLOCK_SIZE) ? BLOCK_SIZE : remaining;
        result = Modbus::EX_GENERAL_FAILURE;
        if (!rtu.writeFileRec(SLAVE_ID, UPDATE_FILE, 0, amount, data, transactionCallback)) {
          updating = false;
          Serial.println("X:send");
          break;
        }
        while (rtu.server()) {
          rtu.task();
          yield();
        }
        remaining -= amount;
        data += amount * 2;
        written += amount;
        if (result != Modbus::EX_SUCCESS) {
          updating = false;
          Serial.println("X");
          break;
        }
        Serial.print(".");
      }
      break;
    case UPLOAD_FILE_END:
      if (!updating) break;
      rtu.writeCoil(SLAVE_ID, UPDATE_ENABLE, false);
      while (rtu.server()) {
        rtu.task();
        yield();
      }
      updating = false;
      Serial.println("!");
      Serial.print("Written: ");
      Serial.println(written * 2);
      break;
    default:
      if (updating) {
        rtu.writeCoil(SLAVE_ID, UPDATE_ENABLE, false);
        while (rtu.server()) {
          rtu.task();
          yield();
        }
        updating = false;
      }
      Serial.print("X");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin("SSID", "PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

#if defined(PICO_RP2040) || defined(PICO_RP2350)
  softwareSerial.begin(115200, SERIAL_8N1);
  rtu.begin(&softwareSerial);
#endif

  web.on("/update", HTTP_POST, updateHandle, updateUploadHandle);
  web.on("/", HTTP_GET, handlePage);
  web.begin();
}

void loop() {
  web.handleClient();
  rtu.task();
  yield();
}
