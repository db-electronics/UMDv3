#pragma once
#include <cstdint>

class FlashInfo{
public:
    uint16_t Manufacturer;
    uint16_t Device;
    uint32_t Size;

    FlashInfo(uint16_t manufacturer, uint16_t device)
        : Manufacturer(manufacturer), Device(device), Size(0)
    {
        switch(Manufacturer){
            // spansion
            case 0x01:
            switch(Device)
                {
                    case 0x7E: // S29GL512N
                    case 0x227E:
                        Size = 0x1000000;
                        break;
                    default:
                        break;
                }
                break;
            // microchip
            case 0xBF:
                switch(Device)
                {
                    case 0x6D: // SST39VF6401B
                    case 0x6C: // SST39VF6402B
                        Size = 0x800000;
                        break;
                    case 0x5D: // SST39VF3201B
                    case 0x5C: // SST39VF3202B
                    case 0x5B: // SST39VF3201
                    case 0x5A: // SST39VF3202
                        Size = 0x400000;
                        break;
                    case 0x4F: // SST39VF1601C
                    case 0x4E: // SST39VF1602C
                    case 0x4B: // SST39VF1601
                    case 0x4A: // SST39VF1602
                        Size = 0x200000;
                        break;
                    default:
                        break;
                }
                break;
            
            // macronix
            case 0xC2:
                switch(Device)
                {
                    case 0xC9: // MX29LV640ET
                    case 0xCB: // MX29LV640EB
                        Size = 0x800000;
                        break;
                    case 0xA7: // MX29LV320ET
                    case 0x22A7:
                    case 0xA8: // MX29LV320EB
                    case 0x22A8:
                        Size = 0x400000;
                        break;
                    case 0xC4: // MX29LV160DT
                    case 0x49: // MX29LV160DB
                        Size = 0x200000;
                        break;    
                    // 5V
                    case 0x58: // MX29F800CT
                    case 0xD6: // MX29F800CB
                        Size = 0x100000;
                        break;
                    case 0x23: // MX29F400CT
                    case 0x2223:
                    case 0xAB: // MX29F400CB
                    case 0x22AB:
                        Size = 0x80000;
                        break;
                    case 0x51: // MX29F200CT
                    case 0x57: // MX29F200CB
                        Size = 0x40000;
                        break;
                    default:
                        break;
                }
                break;
                
            default:
                break;
        }
    }
};