# Bridge functions

## [Basic](basic/basic.ino)

This basic bridge pulls data from a Modbus server and stores it in local registers. A separate Modbus client instance exposes those registers.

## [Modbus RTU-to-TCP bridge](true/true.ino)

Fully functional Modbus RTU-to-TCP bridge.

## [Multiple Server ID](MultipleServerID/MultipleServerID.ino)

Make one Modbus RTU device respond to multiple server identifiers.

## [Modbus TCP-to-RTU simulator](TCP-to-RTU-Simulator/TCP-to-RTU-Simulator.ino)

Fully functional Modbus TCP-to-RTU bridge with an on-device Modbus RTU simulator.

```c
template <typename TargetType>
uint16_t rawRequest(TargetType target, uint8_t* data, uint16_t len, cbTransaction callbackFunction = nullptr, uint8_t unit = MODBUSIP_UNIT);
template <typename TargetType>
uint16_t rawResponse(TargetType target, uint8_t* data, uint16_t len, uint8_t unit = MODBUSIP_UNIT);
template <typename TargetType>
uint16_t errorResponse(TargetType target, Modbus::FunctionCode functionCode, Modbus::ResultCode exceptionCode, uint8_t unit = MODBUSIP_UNIT);
```

- `target`: server identifier (`uint8_t`) or server IP address (`IPAddress`).
- `data`: data buffer to send.
- `len`: number of bytes to send.
- `unit`: unit identifier for Modbus TCP/TLS.
- `functionCode`: function code in the response.
- `exceptionCode`: exception code in the response.

```c
uint16_t setTransactionId(uint16_t transactionIdentifier);
```

- `transactionIdentifier` value that replaces the transaction identifier sequence (Modbus TCP/TLS only)

```c
struct frame_arg_t {
    bool toServer; // True when the frame is a response for the local Modbus server.
    union {
        uint8_t slaveId; // Modbus RTU
        struct {
            uint8_t unitId; // Unit identifier from the Modbus header.
            uint32_t ipAddress; // Source IP address.
            uint16_t transactionId; // Transaction identifier from the Modbus header.
        };
    };
};
typedef std::function<ResultCode(uint8_t*, uint8_t, void*)> cbRaw; // STL callback type.
typedef ResultCode (*cbRaw)(uint8_t* frame, uint8_t len, void* data); // Callback type without the STL.
bool onRaw(cbRaw callbackFunction = nullptr);
```

- `frame` Modbus payload without its MBAP header, server identifier, or CRC
- `len` frame size in bytes
- `data` pointer to a `frame_arg_t` containing frame-header information

Returns:

- `Modbus::EX_PASSTHROUGH` to process the frame normally
- `Modbus::EX_FORCE_PROCESS` to process the frame even when it is addressed to another Modbus unit
- any other code to disable normal frame processing

The callback runs only for a Modbus frame with a valid header and CRC.
