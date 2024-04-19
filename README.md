# UMDv3
The Universal Mega Dumper v3 project is an open-source solution for cartridge dumping and writing. The main PCB includes a very performant MCU, STM32F407VET6, which is connected to a generalized databus consisting of 24 address bits and 16 data lines. Along with a dozen or so control signals, it should be sufficient to interface with most cartridge types. Each cartridge type requires a separate PCB for interface.

To collaborate in the UMDv3 project, please join the [Discord Server](discord.gg/zAUR5jgknm) and come chat.

# Firmware
The firmware is designed to be as accessible as possible and is being developed using [PlatformIO](https://platformio.org/install/ide?install=vscode); a vscode extension which targets many different microcontroller architectures.

Please note that version 3.0 is in its absolute infancy, don't expect to find any working releases yet.

## 2024-04-19
The current firmware has rather solid UI code. It can correctly run a Crc32Mpeg2 id check on Genesis cartridges.

## Uploading
Firmware can be uploaded to the STM32F407 using an STLink programmer or using the built-in DFU bootloader:

- https://dfu-util.sourceforge.net/
- https://sourceforge.net/projects/dfu-util/

# Mainboard
The [UMDv3 Main PCB](https://github.com/db-electronics/UMDv3-kicad) project is a KiCad project PCB that is also fully open source.

# Cartridge Adapters
The UMDv3 by itself only includes a generalized connector with all its interfacing signals. Cartridge adapters must be used in order to interface to the physical media. All cartridge adapters are their own kicad projects in their repositories.

## Cartridge Adapter ID
Each cartridge adapter has an MCP23008 on the I2C bus which identifies the console type for the adapter and allows the UMDv3 to configure its IO accordingly.

- 0x00 - No Adapter / Reserved
- [0x01 - Sega Genesis](https://github.com/db-electronics/UMDv3-genesis-kicad)
- [0x02 - Sega Master System](https://github.com/db-electronics/UMDv3-sms-kicad)
- [0x03 - Super Nintendo](https://github.com/db-electronics/UMDv3-snes-kicad)
- [0x04 - Gameboy Advance](https://github.com/db-electronics/UMDv3-gba-kicad)
- [0x05 - Gameboy / Gameboy Color](https://github.com/db-electronics/UMDv3-gba-kicad)

