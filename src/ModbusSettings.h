#ifndef MODBUS_SETTINGS_H
#define MODBUS_SETTINGS_H

#include <cstddef>
#include <cstdint>

inline constexpr std::size_t MODBUS_MAX_FRAME = 253; // PDU bytes, excluding address and CRC
inline constexpr uint16_t MODBUS_MAX_WORDS = 125;
inline constexpr uint16_t MODBUS_MAX_BITS = 2000;
inline constexpr uint16_t MODBUS_WRITE_WORDS = 123;
inline constexpr uint16_t MODBUS_WRITE_BITS = 1968;

#endif // MODBUS_SETTINGS_H
