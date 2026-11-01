# Embedded integration design (conceptual, simulated)

```text
                   USB (host link)
[Windows host/workbench] ----------------+-----------------------+
        |                                |                       |
        v                                v                       v
 [MCU adapter]                    [FPGA adapter]          [JTAG adapter]
        |                                |                       |
   USB CDC/UART                    USB-JTAG bridge          Host USB
        |                                |                       |
 [MCU board]                      [FPGA board]          [Target JTAG header]
        | GPIO/UART/SPI                  | FPGA I/O               | TCK/TMS/TDI/TDO
        +-------------------------------+-----------------------+
```

| Interface | Power / ground | Signal domain | Connector / protocol | Ownership / assumption |
|---|---|---|---|---|
| Host to MCU | Board powered by its documented USB input; common ground through USB | Example MCU I/O 3.3 V; verify exact board | USB-C/Micro-USB, USB CDC or UART bridge | Host sends commands; MCU owns its GPIO/UART pins |
| Host to FPGA | Board powered by documented USB/barrel input; common ground | Example Basys 3 I/O bank is 3.3 V; verify bank constraints | USB-JTAG onboard bridge | FPGA fabric owns configured user I/O; host configures via supported bridge |
| Host to external JTAG adapter | Adapter USB power; target ground must be connected | Target Vref determines signalling; never assume 3.3 V | Adapter-specific USB plus target JTAG header | Adapter drives TCK/TMS/TDI and samples TDO; target pinout is board-specific |
| Level shifting | Required if voltage domains differ | Match target Vref and absolute maximum ratings | Direction-aware translator where needed | Never connect 5 V signals directly to 3.3 V-only pins |

A JTAG adapter is a debug/programming transport, not a generic compute board: it does not provide a general-purpose processor or FPGA fabric. Board connectors, pin order, Vref and supported protocols differ; consult each exact board manual and schematic before wiring. Use current-limited supplies, verify ground and Vref with a meter, keep power off while wiring, and do not connect two power sources unless the board documentation permits it. The delivered adapter behaviour is mocked; no physical connection is made.
