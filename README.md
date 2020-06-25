TODO
----

- Remove Battery holder from TTGO
- Make bread board to test with voltage regulator, single voltage shifter, pinouts, etc..
- Read canbus messages upon canbus interrupt. store in queue for post-processing.
- Read UART upon intrrupt? UART_RXFIFO_FULL_INT? UART_RXFIFO_TOUT_INT? store in queue for post-processing.
- Log parsed messages directly to flash.
- Add victron BLE module. How to emulate battery monitor?

- Solder serial cable to uCtl

What to log?
---

- Battery Voltage
- Battery Amps
- Battery consumed amps
- Battery %

- Charger DC out

- Solar yield

Write to flash each record.

1 minute window or each value, record the median value.

