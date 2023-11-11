#ifndef UMD_DISPLAY_H
#define UMD_DISPLAY_H

#include <string>
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
#define UMD_DISPLAY_BUFFER_TOTAL_LINES      32


class UMDDisplay
{
    public:
        // scroll[lineIndex][charIndex]

        UMDDisplay();
        bool begin();
        void setCursorChar(char c);
        void setCursorPosition(int x, int y);
        void clear(void);
        void clearLine(int lineNumber);
        void printf(int lineNumber, const char *format, ...);
        void printf(int lineNumber, const __FlashStringHelper *format, ...);
        void print(const char characters[], int lineNumber, int pos);
        void print(const char characters[], int lineNumber);
        void print(int number, int lineNumber);
        void redraw();
        void scrollX(int lineNumber, int delta); // scroll line by delta chars
        void scrollY(int delta); // increment all line numbers by delta
        void scrollY(int delta, int startIndex); // increment all line numbers by delta

    private:
        std::unique_ptr<UMDMenu> _menu;
        static Adafruit_SSD1306 *_display;
        bool _needsRedraw;
        char _cursorChar;
        struct CursorPosition{
            int x;
            int y;
        }_cursorPosition;

        int scroll[OLED_MAX_LINES_PER_SCREEN][2]; // { line index, char index, previous line index }

        int bufferNextPos[UMD_DISPLAY_BUFFER_TOTAL_LINES];
        char buffer[UMD_DISPLAY_BUFFER_TOTAL_LINES][UMD_DISPLAY_BUFFER_CHARS_PER_LINE];
};

class UMDMenu{
    public:
        std::vector<const char *> items;
        
    private:
};

#endif