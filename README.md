# RTOS Task - Low Voltage Learning Phase (FSUK27)

FreeRTOS task for the STM32F103C8 (Blue Pill). A switch interrupt updates a counter, two tasks modify a shared variable, and a UART task prints everything to a PC through a serial terminal (PuTTY).

## Hardware

| Part | Connection |
|---|---|
| Switch / wire | `PA0` to GND (external interrupt, falling edge, internal pull-up) |
| USB-TTL RX | `PA9` (USART1 TX) |
| USB-TTL TX | `PA10` (USART1 RX) |
| USB-TTL GND | GND |
| Programmer | ST-Link (SWD) |

Serial settings: 115200 baud, 8 data bits, no parity, 1 stop bit.

## Files

| File | Description |
|---|---|
| `main.c` | Application code (ISR, tasks, queue, mutex, event group) |
| `FreeRTOSConfig.h` | FreeRTOS configuration (tick = 1 ms) |
| `putty_screenshots/` | PuTTY output screenshots |

## How it works

**Switch ISR** (`HAL_GPIO_EXTI_Callback`)
- Debounces the switch (200 ms, using the FreeRTOS tick).
- Increments the global `counter`, and resets it to 0 when it reaches 9.
- Sets an event bit with `xEventGroupSetBitsFromISR()`.

**Tasks** (all with the same priority)

| Task | Period | What it does |
|---|---|---|
| `CalcSwitch1` | Blocks up to 2 ticks on the event | When the event is set, reads `counter`, multiplies by 10 (the original is not modified) and sends it to the `UARTData` queue |
| `IncreaseVar` | 10 ticks | Increments `shared` (resets to 100 above 200) and sends it to the queue |
| `DecreaseVar` | 12 ticks | Decrements `shared` (resets to 200 below 100) and sends it to the queue |
| `UARTTask` | 5 ticks | Reads the queue and sends `counter: <value>` or `shared: <value>` over UART |

**Synchronization**
- `shared` is protected with a mutex.
- The `UARTData` queue stores a struct with the data source (`counter` or `shared`) and the value.

## Reference comments

- Each HAL API in `main.c` has a comment with its page number in the STM32F1 HAL documentation (UM1850, Rev 3).
- Each FreeRTOS API has a comment with its location in the FreeRTOS website (https://www.freertos.org/).

## Build and run

1. Open the project in STM32CubeIDE and build it.
2. Flash the board with the ST-Link (Run).
3. Open PuTTY (Serial, 115200) on the USB-TTL COM port.
4. Touch `PA0` to GND (or press the switch) and watch the output.

## Expected output

`shared` values stay between 100 and 200, and `counter` values go 10, 20, ... 80, then 0:

```
shared: 132
shared: 133
counter: 60
shared: 132
```

See `putty_screenshots/` for captured output.
