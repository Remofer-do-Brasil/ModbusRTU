# API Modbus RTU

A [interface pública](src/ModbusRTU_PICO.h) declara a configuração da UART, os modos cliente e servidor, as operações locais e as transações remotas. Os códigos de função, exceções e tipos de callback estão no [núcleo do protocolo](src/Modbus.h).

O [guia de integração](README.md) descreve como vincular a biblioteca ao CMake, configurar o transceiver e atender a recepção no loop da aplicação.
