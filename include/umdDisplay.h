#ifndef UMD_DISPLAY_H
#define UMD_DISPLAY_H

#include <string>
#include <vector>
#include <memory>
#include <Adafruit_SSD1306.h>
#include "Menu.h"

#define OLED_RESET -1 
#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64

#define OLED_FONT_WIDTH 6
#define OLED_FONT_HEIGHT 8

#define OLED_MAX_CHARS_PER_LINE         OLED_SCREEN_WIDTH / OLED_FONT_WIDTH
#define OLED_MAX_LINES_PER_SCREEN       OLED_SCREEN_HEIGHT / OLED_FONT_HEIGHT
#define OLED_CHAR_NUMBER(n)             OLED_FONT_WIDTH * n
#define OLED_LINE_NUMBER(n)             OLED_FONT_HEIGHT * n

#define UMD_DISPLAY_BUFFER_CHARS_PER_LINE   (OLED_MAX_CHARS_PER_LINE)*2
#define UMD_DISPLAY_BUFFER_TOTAL_LINES      16
#define UMD_DISPLAY_LAYERS                  2
#define UMD_DISPLAY_LAYER_FG                0
#define UMD_DISPLAY_LAYER_MENU              1

#define UMD_DISPLAY_SCROLL_LINE             0
#define UMD_DISPLAY_SCROLL_CHAR             1

class UMDDisplay
{
    public:
        // scroll[lineIndex][charIndex]

        UMDDisplay();
        bool begin();
        void setCursorChar(char c);
        void setCursorPosition(int x, int y);
        void setCursorVisible(bool visible);
        void setClockPosition(int x, int y);
        void setClockVisible(bool visible);
        void advanceClockAnimation();
        void setLayerLineLength(int layer, int length);
        void clear(void);
        void clearLayer(int layer);
        void clearLine(int layer, int lineNumber);
        void printf(int layer, int lineNumber, const char *format, ...);
        void printf(int layer, int lineNumber, const __FlashStringHelper *format, ...);
        void print(int layer, const char characters[], int lineNumber, int pos);
        void print(int layer, const char characters[], int lineNumber);
        void print(int layer, int number, int lineNumber);
        void redraw(void);

        int showMenu(int layer, UMDMenuIndex menuIndex);
        void initMenu(int layer, const char *menuItems[], int size);
        void initMenu(int layer, const __FlashStringHelper *menuItems[], int size);

        void menuCursorUpdate(int delta, bool visible);
        int menuCurrentItem();

        void scrollX(int layer, int lineNumber, int delta); // scroll line by delta chars
        void scrollY(int layer, int delta); // increment all line numbers by delta

    private:
        
        static Adafruit_SSD1306 *_display;
        bool _needsRedraw;
        int _layerLength[UMD_DISPLAY_LAYERS];
        int _scroll[UMD_DISPLAY_LAYERS][OLED_MAX_LINES_PER_SCREEN][2];
        char _buffer[UMD_DISPLAY_LAYERS][UMD_DISPLAY_BUFFER_TOTAL_LINES][UMD_DISPLAY_BUFFER_CHARS_PER_LINE];
        int _bufferNextPos[UMD_DISPLAY_LAYERS][UMD_DISPLAY_BUFFER_TOTAL_LINES];

        // 0 - MAIN MENU
        #define UMD_MAIN_MENU_SIZE 3
        const __FlashStringHelper* _mainMenuItems[UMD_MAIN_MENU_SIZE] = {F("Read"), F("Write"), F("Test")};
        Menu<UMD_MAIN_MENU_SIZE> _mainMenu = _mainMenuItems;

        // 1 - READ MENU
        #define UMD_READ_MENU_SIZE 4
        const __FlashStringHelper* _readMenuItems[UMD_READ_MENU_SIZE] = {F("Dump ROM"), F("Dump RAM"), F("Header"), F("Flash ID")};
        Menu<UMD_READ_MENU_SIZE> _readMenu = _readMenuItems;

        // 2 - WRITE MENU
        #define UMD_WRITE_MENU_SIZE 2
        const __FlashStringHelper* _writeMenuItems[UMD_WRITE_MENU_SIZE] = {F("Write ROM"), F("Write RAM")};
        Menu<UMD_WRITE_MENU_SIZE> _writeMenu = _writeMenuItems;

        // 3 - Test MENU
        #define UMD_TEST_MENU_SIZE 1
        const __FlashStringHelper* _testMenuItems[UMD_TEST_MENU_SIZE] = {F("Verify Checksum")};
        Menu<UMD_TEST_MENU_SIZE> _testMenu = _testMenuItems;

        struct MenuMetadata
        {
            std::vector<const char *> items;
            bool cursorVisible;
            UMDMenuIndex index;
            int currentItem;
            int scrollRequired;
            int startLine;
            int windowStart;
            int windowEnd;
            int windowSize;
            int layer;
        }_menu;

        void fillLayerFromMenu(int layer, int startBufferIndex, int startMenuIndex);
        void scrollMenu(int delta);
        void initMenuCursor(int layer);
        void setCursorMenuPosition();

        struct Cursor
        {
            int x;
            int y;
            char character;
            bool visible;
        }_cursor;

        #define UMD_CLOCK_ANIMATION_FRAMES 4
        struct Clock
        {
            int framePointer;
            int x;
            int y;
            bool visible;
        }_clock;

        const uint8_t _clockAnimation[UMD_CLOCK_ANIMATION_FRAMES][8] = {
            {
                0b00111000,
                0b01000100,
                0b10010010,
                0b10011010,
                0b10000010,
                0b01000100,
                0b00111000,
                0b00000000
            },
            {
                0b00111000,
                0b01000100,
                0b10000010,
                0b10011010,
                0b10010010,
                0b01000100,
                0b00111000,
                0b00000000
            },
            {
                0b00111000,
                0b01000100,
                0b10000010,
                0b10110010,
                0b10010010,
                0b01000100,
                0b00111000,
                0b00000000
            },
            {
                0b00111000,
                0b01000100,
                0b10010010,
                0b10110010,
                0b10000010,
                0b01000100,
                0b00111000,
                0b00000000
            },
        };
};


#endif