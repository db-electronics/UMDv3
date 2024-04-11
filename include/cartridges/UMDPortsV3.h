#ifndef UMDPORTSV3_H
#define UMDPORTSV3_H

#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_gpio.h>

#define UMD_PORT_ADDR_LOW      GPIOA
#define UMD_PORT_ADDR_MID      GPIOC
#define UMD_PORT_ADDR_HIGH     GPIOD

#define UMD_PORT_DATABUS       GPIOE

#define UMD_PORT_CE0           GPIOC
#define UMD_PIN_CE0            GPIO_PIN_13

#define UMD_PORT_CE1           GPIOD
#define UMD_PIN_CE1            GPIO_PIN_5
#define UMD_PORT_CE2           GPIOD
#define UMD_PIN_CE2            GPIO_PIN_6
#define UMD_PORT_CE3           GPIOD
#define UMD_PIN_CE3            GPIO_PIN_7

#define UMD_PORT_RD            GPIOC
#define UMD_PIN_RD             GPIO_PIN_14
#define UMD_PORT_WR            GPIOC
#define UMD_PIN_WR             GPIO_PIN_15

#define UMD_PORT_IO0           GPIOB
#define UMD_PIN_IO0            GPIO_PIN_0
#define UMD_PORT_IO1           GPIOB
#define UMD_PIN_IO1            GPIO_PIN_1
#define UMD_PORT_IO2           GPIOB
#define UMD_PIN_IO2            GPIO_PIN_5
#define UMD_PORT_IO3           GPIOB
#define UMD_PIN_IO3            GPIO_PIN_6
#define UMD_PORT_IO4           GPIOB
#define UMD_PIN_IO4            GPIO_PIN_13
#define UMD_PORT_IO5           GPIOB
#define UMD_PIN_IO5            GPIO_PIN_12
#define UMD_PORT_IO6           GPIOB
#define UMD_PIN_IO6            GPIO_PIN_15
#define UMD_PORT_IO7           GPIOB
#define UMD_PIN_IO7            GPIO_PIN_14
#define UMD_PORT_IO8           GPIOB
#define UMD_PIN_IO8            GPIO_PIN_7

#define UMD_SWAP_BYTES_16(w)   (w << 8) | (w >> 8)
#define UMD_SWAP_BYTES_32(l)   (((l >> 24) & 0xff) | ((l << 8) & 0xff0000) | ((l >> 8) & 0xff00) | ((l << 24) & 0xff000000))

#define M_REPEAT_5(X)          X X X X X
#define M_REPEAT_10(X)         M_REPEAT_5(X) M_REPEAT_5(X)
#define M_REPEAT_20(X)         M_REPEAT_10(X) M_REPEAT_10(X)
#define M_REPEAT_30(X)         M_REPEAT_20(X) M_REPEAT_10(X)
#define M_REPEAT_40(X)         M_REPEAT_20(X) M_REPEAT_20(X)

// these wait numbers may seem arbitrary but they were measured IRL
#define M_REPEAT_8(X)          M_REPEAT_5(X) X X X
#define M_REPEAT_16(X)         M_REPEAT_10(X) M_REPEAT_5(X) X
#define M_REPEAT_24(X)         M_REPEAT_20(X) X X X X
#define M_REPEAT_33(X)         M_REPEAT_30(X) X X X
#define M_REPEAT_41(X)         M_REPEAT_40(X) X

/// @brief UMDPorts defines protected methods which abstract the hardware interface to the cartridge 
class UMDPortsV3
{
    protected:
        /// @brief configure the default state of catridge IO
        void setDefaults();

        /// @brief nop sequence to wait approximately 50ns
        __attribute__((always_inline)) void wait50ns() { asm volatile(M_REPEAT_8("nop\n\r")); }

        /// @brief nop sequence to wait approximately 100ns
        __attribute__((always_inline)) void wait100ns() { asm volatile(M_REPEAT_16("nop\n\r")); }

        /// @brief nop sequence to wait approximately 150ns
        __attribute__((always_inline)) void wait150ns() { asm volatile(M_REPEAT_24("nop\n\r")); }

        /// @brief nop sequence to wait approximately 200ns
        __attribute__((always_inline)) void wait200ns() { asm volatile(M_REPEAT_33("nop\n\r")); }

        /// @brief nop sequence to wait approximately 250ns
        __attribute__((always_inline)) void wait250ns() { asm volatile(M_REPEAT_41("nop\n\r")); }

        /// @brief write a 16 bit address to the address bus
        /// @param address 16 bit address
        __attribute__((always_inline)) void addressWrite(uint16_t address)
        {
            _portByteWriteLow(UMD_PORT_ADDR_LOW, (uint8_t)(0xFF & address));
            _portByteWriteLow(UMD_PORT_ADDR_MID, (uint8_t)(0xFF & address >> 8));
        }

        /// @brief write a 24 bit address to the address bus
        /// @param address 24 bit address
        __attribute__((always_inline)) void addressWrite(uint32_t address)
        {
            _portByteWriteLow(UMD_PORT_ADDR_LOW, (uint8_t)(0xFF & address));
            _portByteWriteLow(UMD_PORT_ADDR_MID, (uint8_t)(0xFF & address >> 8));
            _portByteWriteHigh(UMD_PORT_ADDR_HIGH, (uint8_t)(0xFF & address >> 16));
        }

        /// @brief read the lower 8 bits of the data bus
        /// @return data
        __attribute__((always_inline)) uint8_t dataReadLow() { return _portByteReadLow(UMD_PORT_DATABUS); }

        /// @brief read the upper 8 bits of the data bus
        /// @return data
        __attribute__((always_inline)) uint8_t dataReadHigh() { return _portByteReadHigh(UMD_PORT_DATABUS); }

        /// @brief read a little endian word from the data bus
        /// @return little endian word
        __attribute__((always_inline)) uint16_t dataReadWord() { return _portWordRead(UMD_PORT_DATABUS); }

        /// @brief read a big endian word from the data bus
        /// @return big endian word
        __attribute__((always_inline)) uint16_t dataReadWordSwapped()
        {
            return UMD_SWAP_BYTES_16(_portWordRead(UMD_PORT_DATABUS));
        }

        /// @brief write a byte to the data bus
        /// @param value
        __attribute__((always_inline)) void dataWrite(uint8_t value) { _portByteWriteLow(UMD_PORT_DATABUS, value); }

        /// @brief write a byte to the lower 8 bits of the data bus
        /// @param value
        __attribute__((always_inline)) void dataWriteLow(uint8_t value) { _portByteWriteLow(UMD_PORT_DATABUS, value); }

        /// @brief write a byte to the upper 8 bits of the data bus
        /// @param value
        __attribute__((always_inline)) void dataWriteHigh(uint8_t value)
        {
            _portByteWriteHigh(UMD_PORT_DATABUS, value);
        }

        /// @brief write a little endian word to the data bus
        /// @param value
        __attribute__((always_inline)) void dataWrite(uint16_t value)
        {
            _portWordWrite(UMD_PORT_DATABUS, value);
        }

        /// @brief write a big endian word to the data bus
        /// @param value
        __attribute__((always_inline)) void dataWriteSwapped(uint16_t value)
        {
            _portWordWrite(UMD_PORT_DATABUS, UMD_SWAP_BYTES_16(value));
        }

        /// @brief set the data bus to inputs
        /// @param pullups activate internal pullups?
        __attribute__((always_inline)) void dataSetToInputs(bool pullups)
        {
            _portSetToInput(UMD_PORT_DATABUS, pullups);
        }

        /// @brief set the data bus to outputs
        __attribute__((always_inline)) void dataSetToOutputs() { _portSetToOutput(UMD_PORT_DATABUS); }

        /// @brief set the CE0 pin
        __attribute__((always_inline)) void setCE0() { _bitSet(UMD_PORT_CE0, UMD_PIN_CE0); }

        /// @brief set the CE1 pin
        __attribute__((always_inline)) void setCE1() { _bitSet(UMD_PORT_CE1, UMD_PIN_CE1); }

        /// @brief set the CE2 pin
        __attribute__((always_inline)) void setCE2() { _bitSet(UMD_PORT_CE2, UMD_PIN_CE2); }

        /// @brief set the CE3 pin
        __attribute__((always_inline)) void setCE3() { _bitSet(UMD_PORT_CE3, UMD_PIN_CE3); }

        /// @brief set the WR pin
        __attribute__((always_inline)) void setWR() { _bitSet(UMD_PORT_WR, UMD_PIN_WR); }

        /// @brief set the RD pin
        __attribute__((always_inline)) void setRD() { _bitSet(UMD_PORT_RD, UMD_PIN_RD); }

        /// @brief clear the CE0 pin
        __attribute__((always_inline)) void clearCE0() { _bitClear(UMD_PORT_CE0, UMD_PIN_CE0); }

        /// @brief clear the CE1 pin
        __attribute__((always_inline)) void clearCE1() { _bitClear(UMD_PORT_CE1, UMD_PIN_CE1); }

        /// @brief clear the CE2 pin
        __attribute__((always_inline)) void clearCE2() { _bitClear(UMD_PORT_CE2, UMD_PIN_CE2); }

        /// @brief clear the CE3 pin
        __attribute__((always_inline)) void clearCE3() { _bitClear(UMD_PORT_CE3, UMD_PIN_CE3); }

        /// @brief clear the RD pin
        __attribute__((always_inline)) void clearRD() { _bitClear(UMD_PORT_RD, UMD_PIN_RD); }

        /// @brief clear the WR pin
        __attribute__((always_inline)) void clearWR() { _bitClear(UMD_PORT_WR, UMD_PIN_WR); }

        /// @brief set an IO pin
        /// @param io IO pin number 0-8
        __attribute__((always_inline)) void setIO(const uint8_t io)
        {
            switch (io)
            {
            case 0: _bitSet(UMD_PORT_IO0, UMD_PIN_IO0); break;
            case 1: _bitSet(UMD_PORT_IO1, UMD_PIN_IO1); break;
            case 2: _bitSet(UMD_PORT_IO2, UMD_PIN_IO2); break;
            case 3: _bitSet(UMD_PORT_IO3, UMD_PIN_IO3); break;
            case 4: _bitSet(UMD_PORT_IO4, UMD_PIN_IO4); break;
            case 5: _bitSet(UMD_PORT_IO5, UMD_PIN_IO5); break;
            case 6: _bitSet(UMD_PORT_IO6, UMD_PIN_IO6); break;
            case 7: _bitSet(UMD_PORT_IO7, UMD_PIN_IO7); break;
            case 8: _bitSet(UMD_PORT_IO8, UMD_PIN_IO8); break;
            default: break;
            }
        }

        /// @brief clean an IO pin
        /// @param io IO pin number 0-8
        __attribute__((always_inline)) void clearIO(const uint8_t io)
        {
            switch (io)
            {
            case 0: _bitClear(UMD_PORT_IO0, UMD_PIN_IO0); break;
            case 1: _bitClear(UMD_PORT_IO1, UMD_PIN_IO1); break;
            case 2: _bitClear(UMD_PORT_IO2, UMD_PIN_IO2); break;
            case 3: _bitClear(UMD_PORT_IO3, UMD_PIN_IO3); break;
            case 4: _bitClear(UMD_PORT_IO4, UMD_PIN_IO4); break;
            case 5: _bitClear(UMD_PORT_IO5, UMD_PIN_IO5); break;
            case 6: _bitClear(UMD_PORT_IO6, UMD_PIN_IO6); break;
            case 7: _bitClear(UMD_PORT_IO7, UMD_PIN_IO7); break;
            case 8: _bitClear(UMD_PORT_IO8, UMD_PIN_IO8); break;
            default: break;
            }
        }

        /// @brief read an IO pin
        /// @param io IO pin number 0-8
        /// @return 0 or 1
       __attribute__((always_inline)) uint8_t ioRead(const uint8_t io)
       {
            switch(io){
                case 0: return _bitRead(UMD_PORT_IO0, UMD_PIN_IO0);
                case 1: return _bitRead(UMD_PORT_IO1, UMD_PIN_IO1);
                case 2: return _bitRead(UMD_PORT_IO2, UMD_PIN_IO2);
                case 3: return _bitRead(UMD_PORT_IO3, UMD_PIN_IO3);
                case 4: return _bitRead(UMD_PORT_IO4, UMD_PIN_IO4);
                case 5: return _bitRead(UMD_PORT_IO5, UMD_PIN_IO5);
                case 6: return _bitRead(UMD_PORT_IO6, UMD_PIN_IO6);
                case 7: return _bitRead(UMD_PORT_IO7, UMD_PIN_IO7);
                case 8: return _bitRead(UMD_PORT_IO8, UMD_PIN_IO8);
                default: return 0;
            }
            return 0;
       }

        /// @brief configure an IO pin as an output
        /// @param io IO pin number 0-8
        /// @param pushpull pushpull or open drain (default = true)
        void ioSetToOutput(uint16_t io, bool pushpull);

        /// @brief configure an IO pin as an input
        /// @param io IO pin number 0-8
        /// @param pullup pullup resistor active (default = false)
        void ioSetToInput(uint16_t io, bool pullup);

    private:
        uint16_t ticks;

        /// @brief fast inline bit set on port
        /// @param GPIOx GPIO_TypeDef
        /// @param GPIO_Pin GPIO pin number 1 to 16
        __attribute__((always_inline)) void _bitSet(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) { GPIOx->BSRR = GPIO_Pin; }

        /// @brief fast inline bit clear on port
        /// @param GPIOx GPIO_TypeDef
        /// @param GPIO_Pin GPIO pin number 1 to 16
        __attribute__((always_inline)) void _bitClear(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
        {
            GPIOx->BSRR = (uint32_t)GPIO_Pin << 16U;
        }

        /// @brief fast inline bit read on port
        /// @param GPIOx GPIO_TypeDef
        /// @param GPIO_Pin GPIO pin number 1 to 16
        /// @return 0 or 1
        __attribute__((always_inline)) uint8_t _bitRead(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
        {
            return GPIOx->IDR & GPIO_Pin != 0 ? 1 : 0;
        }

        /// @brief configure an IO pin as an output
        /// @param GPIOx GPIO_TypeDef
        /// @param GPIO_Pin GPIO pin number 1 to 16
        /// @param pushpull pushpull (true) or open drain (false) output (default = true)
        void _bitSetToOutput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, bool pushpull);

        /// @brief configure an IO pin as an input
        /// @param GPIOx GPIO_TypeDef
        /// @param GPIO_Pin GPIO pin number 1 to 16
        /// @param pullup activate pullup resistor (default = false)
        void _bitSetToInput(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, bool pullup);

        /// @brief write a byte to the lower 8 bits of a port
        /// @param GPIOx GPIO_TypeDef
        /// @param value byte
        __attribute__((always_inline)) void _portByteWriteLow(GPIO_TypeDef *GPIOx, uint8_t value)
        {
            uint16_t portValue = GPIOx->IDR; // don't affect the upper 8 bits
            portValue &= 0xFF00;
            portValue |= (uint16_t)(value);
            GPIOx->ODR = portValue;
        }

        /// @brief read a byte from the lower 8 bits of a port
        /// @param GPIOx GPIO_TypeDef
        /// @return byte
        __attribute__((always_inline)) uint8_t _portByteReadLow(GPIO_TypeDef *GPIOx)
        {
            return (uint8_t)(GPIOx->IDR & 0xFF);
        }

        /// @brief write a byte to the upper 8 bits of a port
        /// @param GPIOx GPIO_TypeDef
        /// @param value byte
        __attribute__((always_inline)) void _portByteWriteHigh(GPIO_TypeDef *GPIOx, uint8_t value)
        {
            uint16_t portValue = GPIOx->IDR; // don't affect the lower 8 bits
            portValue &= 0x00FF;
            portValue |= (uint16_t)(value << 8);
            GPIOx->ODR = portValue;
        }

        /// @brief read a byte from the upper 8 bits of a port
        /// @param GPIOx GPIO_TypeDef
        /// @return byte
        __attribute__((always_inline)) uint8_t _portByteReadHigh(GPIO_TypeDef *GPIOx)
        {
            return (uint8_t)((GPIOx->IDR >> 8) & 0xFF);
        }

        /// @brief write a word to the full 16 bits of a port
        /// @param GPIOx GPIO_TypeDef
        /// @param value word
        __attribute__((always_inline)) void _portWordWrite(GPIO_TypeDef *GPIOx, uint16_t value) { GPIOx->ODR = value; }

        /// @brief read a full 16 bit word from a port
        /// @param GPIOx GPIO_TypeDef
        /// @return word
        __attribute__((always_inline)) uint16_t _portWordRead(GPIO_TypeDef *GPIOx) { return (uint16_t)GPIOx->IDR; }

        /// @brief configure an entire 16 bit port as inputs
        /// @param GPIOx GPIO_TypeDef
        /// @param pullups active pullup resistors
        __attribute__((always_inline)) void _portSetToInput(GPIO_TypeDef *GPIOx, bool pullups)
        {
            GPIOx->MODER = 0x00000000; // 0b00 per pin group for input
            if (pullups)
            {
                GPIOx->PUPDR = 0x55555555; // 0b01 per pin group for pull-up
            }
        }

        /// @brief configure an entire 16 bit port as ouputs
        /// @param GPIOx GPIO_TypeDef
        __attribute__((always_inline)) void _portSetToOutput(GPIO_TypeDef *GPIOx)
        {
            GPIOx->MODER = 0x55555555; // 0b01 per pin group for General Purpose Output Mode
        }
};

#endif