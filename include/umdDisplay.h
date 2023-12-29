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
        void clear(void);
        void clearLine(int layer, int lineNumber);
        void printf(int layer, int lineNumber, const char *format, ...);
        void printf(int layer, int lineNumber, const __FlashStringHelper *format, ...);
        void print(int layer, const char characters[], int lineNumber, int pos);
        void print(int layer, const char characters[], int lineNumber);
        void print(int layer, int number, int lineNumber);
        void initMenu(int layer, const char *menuItems[], int size);
        void initMenu(int layer, const __FlashStringHelper *menuItems[], int size);
        void redraw(void);

        void scrollX(int lineNumber, int delta); // scroll line by delta chars
        void scrollY(int delta); // increment all line numbers by delta
        void scrollY(int delta, int startIndex); // increment all line numbers by delta

    private:
        std::vector<const char *> _menu;
        int _menuItemPtr;

        static Adafruit_SSD1306 *_display;
        bool _needsRedraw;
        char _cursorChar;
        struct CursorPosition{
            int x;
            int y;
        }_cursorPosition;

        void fillLayerFromMenu(int layer, int startBufferIndex, int startMenuIndex);

        int scroll[OLED_MAX_LINES_PER_SCREEN][2]; // { line index, char index, previous line index }
        int bufferNextPos[UMD_DISPLAY_BUFFER_TOTAL_LINES];

        char buffer[UMD_DISPLAY_BUFFER_TOTAL_LINES][UMD_DISPLAY_BUFFER_CHARS_PER_LINE];

        int _layerLength[UMD_DISPLAY_LAYERS];
        int _scroll[UMD_DISPLAY_LAYERS][OLED_MAX_LINES_PER_SCREEN][2];
        char _buffer[UMD_DISPLAY_LAYERS][UMD_DISPLAY_BUFFER_TOTAL_LINES][UMD_DISPLAY_BUFFER_CHARS_PER_LINE];
        int _bufferNextPos[UMD_DISPLAY_LAYERS][UMD_DISPLAY_BUFFER_TOTAL_LINES];
};


#endif