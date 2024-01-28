#ifndef UMD_DISPLAY_H
#define UMD_DISPLAY_H

#include <string>
#include <vector>
#include <memory>
#include <Adafruit_SSD1306.h>

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
#define UMD_DISPLAY_LAYER_BG                1

#define UMD_DISPLAY_SCROLL_LINE             0
#define UMD_DISPLAY_SCROLL_CHAR             1

// template <size_t menuSize>
// class UMDMenu{
//     public:
//         const char* menuItems[menuSize]
//         size_t count;

//         UMDMenu() : itemCount(0){}

//         UMDMenu(const char* items[], size_t count) : itemCount(0) {
//             for(size_t i = 0; i < count && i < menuSize; i++){
//                 menuItems[i] = items[i];
//                 itemCount++;
//             }
//         }
// };

// UMDMenu topLevelMenu{

// };

// const __FlashStringHelper * menuTopLevel[] = {F("Read Cartridge"), F("Write Cartridge")};

class UMDDisplay
{
    public:
        // scroll[lineIndex][charIndex]

        UMDDisplay();
        bool begin();
        void setCursorChar(char c);
        void setCursorPosition(int x, int y);
        void setClockPosition(int x, int y);
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

        void initMenu(int layer, const char *menuItems[], int size);
        void initMenu(int layer, const __FlashStringHelper *menuItems[], int size);
        void menuCursorUpdate(int delta, bool visible);
        int menuCurrentItem();

        void scrollX(int layer, int lineNumber, int delta); // scroll line by delta chars
        void scrollY(int layer, int delta); // increment all line numbers by delta

    private:
        
        struct Menu
        {
            std::vector<const char *> items;
            bool cursorVisible;
            int scrollRequired;
            int currentItem;
            int startLine;
            int windowStart;
            int windowEnd;
            int windowSize;
            int layer;
        }_menu;

        static Adafruit_SSD1306 *_display;
        bool _needsRedraw;

        struct Cursor
        {
            int x;
            int y;
            char character;
        }_cursor;

        #define UMD_CLOCK_ANIMATION_FRAMES 4
        struct Clock
        {
            int framePointer;
            int x;
            int y;
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

        void fillLayerFromMenu(int layer, int startBufferIndex, int startMenuIndex);
        void scrollMenu(int delta);
        void initMenuCursor(int layer);
        void setCursorMenuPosition();

        int _layerLength[UMD_DISPLAY_LAYERS];
        int _scroll[UMD_DISPLAY_LAYERS][OLED_MAX_LINES_PER_SCREEN][2];
        char _buffer[UMD_DISPLAY_LAYERS][UMD_DISPLAY_BUFFER_TOTAL_LINES][UMD_DISPLAY_BUFFER_CHARS_PER_LINE];
        int _bufferNextPos[UMD_DISPLAY_LAYERS][UMD_DISPLAY_BUFFER_TOTAL_LINES];
};


#endif