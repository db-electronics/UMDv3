#ifndef MENU_H
#define MENU_H

template<int size>
class Menu
{
    public:
        const int Size = size;
        const __FlashStringHelper* Items[size];
        Menu(const __FlashStringHelper* items[size])
        {
            for(int i = 0; i < size; i++)
            {
                Items[i] = items[i];
            }
        }
};

enum UMDMenuIndex : int{
        UMD_NOCHANGE = -1,
        UMD_MAIN = 0,
        UMD_READ,
        UMD_WRITE,
        UMD_TEST
    };
#endif