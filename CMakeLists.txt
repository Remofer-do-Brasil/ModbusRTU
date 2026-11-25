add_library(ModbusRTU STATIC src/Modbus.cpp src/ModbusAPI.cpp src/ModbusRTU.cpp)
target_compile_features(ModbusRTU PUBLIC cxx_std_17 PRIVATE cxx_std_20)
target_include_directories(ModbusRTU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ModbusRTU
  PRIVATE hardware_uart_headers hardware_gpio_headers pico_time_headers
  INTERFACE hardware_uart hardware_gpio pico_time
)

add_executable(modbus_rtu_server EXCLUDE_FROM_ALL examples/rtu_server/main.cpp)
target_link_libraries(modbus_rtu_server PRIVATE ModbusRTU pico_stdlib)
pico_enable_stdio_uart(modbus_rtu_server 0)
pico_enable_stdio_usb(modbus_rtu_server 0)
pico_add_extra_outputs(modbus_rtu_server)

add_custom_target(rtu_server_example DEPENDS modbus_rtu_server)
