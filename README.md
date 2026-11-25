# ModbusRTU para Raspberry Pi Pico SDK

Biblioteca C++ de cliente e servidor Modbus RTU sobre UART de hardware. O mapa de coils, entradas e registradores pertence a cada instância e cresce
conforme a memória disponível. A recepção, a resposta e a requisição pendente usam buffers de tamanho fixo. Um quadro RTU comporta até 256 bytes,
incluindo endereço e CRC.

## Integração CMake

Após inicializar o Pico SDK com `pico_sdk_init()`, adicione a biblioteca ao projeto e vincule o alvo ao executável.

```cmake
add_subdirectory(lib/ModbusRTU)
target_link_libraries(meu_firmware PRIVATE ModbusRTU)
```

Inclua `ModbusRTU_PICO.h`. Reserve uma UART para a instância durante toda a operação. O aplicativo deve escolher os GPIOs conforme o esquema da placa
e desativar o stdio nessa UART. A configuração recebe TX, RX, baud rate, paridade e um GPIO opcional de direção. O padrão é 8E1. Paridade ímpar
seleciona 8O1 e ausência de paridade seleciona 8N2. Para RS-485, ligue DE e /RE ao mesmo GPIO de direção ou configure /RE separadamente com
`receiverEnablePin`. Transceivers com direção automática dispensam esses GPIOs.

## Servidor

O exemplo para uma **Pico 2 de bancada** usa UART0, GP0 (TX), GP1 (RX), 19200 bit/s e endereço 1. Ele expõe 125 holding registers a partir do endereço
PDU 0, inicialmente com o valor 0x1234. Os pinos dos produtos RM-SG01 dependem da placa. [Exemplo completo](examples/rtu_server/main.cpp)

```sh
cmake --build --preset ecu-test-iii -t rtu_server_example
```

### Remoção de callbacks por ID

Use `onGetWithId` ou `onSetWithId` para obter o ID de um registro de callback. O ID pertence à instância e cobre todos os endereços atendidos pela
chamada. Se nenhum callback for registrado, o retorno é `std::nullopt`.

```cpp
auto id = modbus.onSetWithId(HREG(0), callback, 2);
// Quando o callback deixar de ser necessário:
if (id) {
  modbus.removeCallback(*id);
}
```

`removeCallback` retorna `true` se removeu alguma entrada. Para remover todos os callbacks de um tipo por endereço, use `removeOnSet(address)` ou
`removeOnGet(address)`.

## Cliente

Após `begin(config)`, chame `client()` e use métodos como `readHreg(id, offset, valores, quantidade, callback)`. O retorno 1 indica envio. Mantenha o
buffer de destino válido até a conclusão da transação. O cliente aceita uma requisição pendente por vez e informa o resultado pelo callback, inclusive
timeout após um segundo. Escritas broadcast usam o endereço 0 e terminam no envio.

Chame `task()` continuamente a partir de um único contexto. A UART fornece FIFO sem timestamps por byte. Para observar as pausas entre caracteres,
atenda a recepção em intervalos bem menores que um caractere (cerca de 573 µs a 19200 bit/s). A transmissão é síncrona até a saída do último stop bit.
Evite bloqueios longos no loop e nos callbacks. Configure o mapa e os callbacks fora do processamento de quadros.

As leituras suportam até 2000 bits ou 125 registradores por requisição. Escritas múltiplas suportam até 1968 coils ou 123 registradores. A função
combinada 0x17 permite ler 125 e escrever 121 registradores na mesma transação.

Derivada de sndnvaps/modbus-pico e do trabalho de Alexander Emelianov e André Sarmento Barbosa. [Licença BSD](LICENSE.txt)
