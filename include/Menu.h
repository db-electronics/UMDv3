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
        UMD_MENU_MAIN = 0,
        UMD_MENU_MEMORIES,
        UMD_MENU_READ,
        UMD_MENU_WRITE,
        UMD_MENU_TEST
    };

#endif