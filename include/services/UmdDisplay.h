#pragma once

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

#define UMD_DISPLAY_BUFFER_CHARS_PER_LINE   ((OLED_MAX_CHARS_PER_LINE)*2)
#define UMD_DISPLAY_BUFFER_TOTAL_LINES      16
#define UMD_DISPLAY_WINDOW_SIZE             6
#define UMD_DISPLAY_LAYERS                  2
#define UMD_DISPLAY_LAYER_FG                0
#define UMD_DISPLAY_LAYER_MENU              1

#define UMD_DISPLAY_SCROLL_LINE             0
#define UMD_DISPLAY_SCROLL_CHAR             1

class UMDDisplay
{
    public:

        UMDDisplay();
        
        
        void setClockPosition(int x, int y);
        void setClockVisible(bool visible);
        void advanceClockAnimation();

        void clear(void);
        void clearLayer(int layer);
        void clearLine(int layer, int lineNumber);
        void printf(int layer, int lineNumber, const char *format, ...);
        void printf(int layer, int lineNumber, const __FlashStringHelper *format, ...);
        void print(int layer, const char characters[], int lineNumber, int pos);
        void print(int layer, const char characters[], int lineNumber);
        void print(int layer, int number, int lineNumber);
        void redraw(void);


        void LoadMenuItems(int layer, std::vector<const char *>& items);
        void LoadMenuItems(int layer, const char *items[], int size);

        void menuCursorUpdate(int delta, bool visible);
        uint8_t GetCurrentItemIndex();

        
        void scrollX(int layer, int lineNumber, int delta); // scroll line by delta chars
        void scrollY(int layer, int delta); // increment all line numbers by delta

        // persistent buffer to add to the display
        void ClearScratchBufferLine(int lineNumber);
        char ScratchBuffer[UMD_DISPLAY_BUFFER_TOTAL_LINES][UMD_DISPLAY_BUFFER_CHARS_PER_LINE];

        enum Zone : uint8_t
        {
            ZONE_TITLE = 0,
            ZONE_STATUS,
            ZONE_WINDOW
        };

        bool Init();
        void SetZoneVisibility(Zone zone, bool visible);

        void ClearZone(Zone zone);
        void ClearLine(Zone zone, uint8_t lineNumber);

        void Printf(Zone zone, const __FlashStringHelper *format, ...);
        void SetWindowItems(const std::vector<const char *>& items);
        void SetCursorChar(char c);
        void SetCursorVisibility(bool visible);
        void SetCursorPosition(int x, int y, bool wrap = false);
        void IncCursorItemPosition();
        void DecCursorItemPosition();

        void IncWindowScrollX(uint8_t lineNumber);
        void DecWindowScrollX(uint8_t lineNumber);
        void ResetScrollX();
        void IncWindowScrollY();
        void DecWindowScrollY();

        void Redraw(void);

    private:
        
        static Adafruit_SSD1306 *_display;
        bool _needsRedraw;
        int _layerLength[UMD_DISPLAY_LAYERS];
        int _scroll[UMD_DISPLAY_LAYERS][OLED_MAX_LINES_PER_SCREEN][2];
        char _buffer[UMD_DISPLAY_LAYERS][UMD_DISPLAY_BUFFER_TOTAL_LINES][UMD_DISPLAY_BUFFER_CHARS_PER_LINE];
        int _bufferNextPos[UMD_DISPLAY_LAYERS][UMD_DISPLAY_BUFFER_TOTAL_LINES];

        // character buffers for the display
        bool mRedrawScreen;
        
        bool mTitleVisible;
        bool mWindowVisible;
        bool mStatusVisible;

        uint8_t mWindowCurrentLine;
        uint8_t mUserCurrentLine;

        // line 0 is the title
        std::array<char, OLED_MAX_CHARS_PER_LINE+1> mTitleBuffer; // +1 for terminator
        // final line is the status
        std::array<char, OLED_MAX_CHARS_PER_LINE+1> mStatusBuffer;
        // lines 1-n are the scrollable window
        std::array<std::array<char, UMD_DISPLAY_BUFFER_CHARS_PER_LINE+1>, UMD_DISPLAY_BUFFER_TOTAL_LINES> mWindowBuffer;
        // window scroll x position, indicates on which character the window starts
        std::array<uint8_t, UMD_DISPLAY_BUFFER_TOTAL_LINES> mWindowScrollX;
        
        std::array<char, OLED_MAX_CHARS_PER_LINE+1> mLineBuffer;
        // window scroll y position, indicates on which line the window starts
        uint8_t mWindowScrollY;

        void LoadWindowItemsToBuffer(uint8_t startItemIndex, uint8_t startBufferIndex, uint8_t windowSize);
        uint8_t GetWindowVisibleLinesCount();

        struct WindowItemsData{
            std::vector<const char *> items;
            uint8_t StartIndex;
            uint8_t SelectedItemIndex;
            uint8_t WindowSize;
            uint8_t WindowStart;
            uint8_t CursorIndex;
            uint8_t Count;   // items added to buffer afterwards
            
            void Reset(uint8_t startIndex, uint8_t windowSize){
                StartIndex = startIndex;
                SelectedItemIndex = 0;
                WindowSize = windowSize;
                WindowStart = 0;
                CursorIndex = 0;
                Count = 0;
            }
        }mWindowItems;

        struct MenuMetadata
        {
            std::vector<const char *> items;
            bool cursorVisible;
            uint8_t currentItem;
            int scrollRequired;
            int startLine;
            int windowStart;
            int windowEnd;
            int windowSize;
            int layer;

        }mMenu;

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
        }mCursor;

        #define UMD_CLOCK_ANIMATION_FRAMES 4
        struct Clock
        {
            int framePointer;
            int x;
            int y;
            bool visible;
        }mClockSpr;

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
