#ifndef MCP23008_H
#define MCP23008_H



#define MCP23008_ADDRESS    0x27 //!< MCP23008 serial address
// registers
#define MCP23008_IODIR      0x00 //!< I/O direction register
#define MCP23008_IPOL       0x01 //!< Input polarity register
#define MCP23008_GPINTEN    0x02 //!< Interrupt-on-change control register
#define MCP23008_DEFVAL     0x03 //!< Default compare register for interrupt-on-change
#define MCP23008_INTCON     0x04 //!< Interrupt control register
#define MCP23008_IOCON      0x05 //!< Configuration register
#define MCP23008_GPPU       0x06 //!< Pull-up resistor configuration register
#define MCP23008_INTF       0x07 //!< Interrupt flag register
#define MCP23008_INTCAP     0x08 //!< Interrupt capture register
#define MCP23008_GPIO       0x09 //!< Port register
#define MCP23008_OLAT       0x0A //!< Output latch register

#endif