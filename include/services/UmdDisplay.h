#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
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
#define UMD_DISPLAY_BUFFER_TOTAL_LINES      8
#define UMD_DISPLAY_WINDOW_SIZE             6
#define UMD_DISPLAY_LAYERS                  2
#define UMD_DISPLAY_LAYER_FG                0
#define UMD_DISPLAY_LAYER_MENU              1


class UMDDisplay
{
    public:

        enum Zone : uint8_t
        {
            ZONE_TITLE = 0,
            ZONE_STATUS,
            ZONE_WINDOW
        };

        UMDDisplay(std::unique_ptr<Adafruit_SSD1306> display);

        uint8_t GetSelectedItemIndex();


        bool Init();
        void SetZoneVisibility(Zone zone, bool visible);

        void ClearZone(Zone zone);
        void ClearLine(Zone zone, uint8_t lineNumber);

        /// @brief print a line directly to the output buffer, n.b. be wary of buffer line size
        /// @param zone the zone to print to 
        /// @param format the format string
        /// @param args the arguments to the format string
        void Printf(Zone zone, const __FlashStringHelper *format, ...);

        /// @brief load a set of items of arbitrary length into the window buffer, this clears previous items
        /// @param items 
        void SetWindowItems(const std::vector<const char *>& items);
        
        /// @brief clear current window items
        void ClearWindowItems();
        void UpdateCursorItemPosition(int delta);
        void SetWindowItemScrollX(int delta);


        void SetCursorChar(char c);
        void SetCursorVisibility(bool visible);
        void SetCursorPosition(int x, int y);

        void ResetScrollX();
        void SetWindowScrollY(int delta);

        void Redraw(void);

    private:
        
        std::unique_ptr<Adafruit_SSD1306> mDisplay;
        
        struct FontInfo
        {
            uint8_t Width;
            uint8_t Height;
            
            void Set(uint8_t width, uint8_t height)
            {
                Width = width;
                Height = height;
            }
        } mFontInfo;

        bool mRedrawScreen;

        bool mTitleVisible;
        bool mWindowVisible;
        bool mStatusVisible;

        int mWindowCurrentLine;
        int mUserCurrentLine;

        // line 0 is the title
        std::array<char, OLED_MAX_CHARS_PER_LINE+1> mTitleBuffer; // +1 for terminator
        // final line is the status
        std::array<char, OLED_MAX_CHARS_PER_LINE+1> mStatusBuffer;
        // lines 1-n are the scrollable window
        std::array<std::array<char, UMD_DISPLAY_BUFFER_CHARS_PER_LINE+1>, UMD_DISPLAY_BUFFER_TOTAL_LINES> mWindowBuffer;
        
        std::array<char, OLED_MAX_CHARS_PER_LINE+1> mLineBuffer;

        // window scroll y position, indicates on which line the window starts
        int mWindowScrollY;
        // window scroll x position, indicates on which character the window starts
        std::array<int, UMD_DISPLAY_BUFFER_TOTAL_LINES> mWindowScrollX;

        void LoadWindowItemsToBuffer();
        void LoadWindowItemToBuffer(int itemIndex, int bufferIndex);
        int GetWindowVisibleLinesCount();

        struct WindowItemsData{
            std::vector<const char *> items;
            int StartLine;          // display line where the window starts
            int SelectedItemIndex;  // index of the selected item in the display buffer
            int WindowSize;         // number of items that can be displayed in the window
            int WindowStart;        // index of the first item in the visible window
            int WindowEnd;          // index of the last item in the visible window    
            int TotalItems;         // items can be added to the display buffer afterwards
            int StartBufferItem;    // index of the first item in the display buffer
            int EndBufferItem;      // index of the last item in the display buffer
            int ScrollRequired;

            void Reset(int startLine, int windowSize, const std::vector<const char *>& newItems){
                items.assign(newItems.begin(), newItems.end());
                StartLine = startLine;
                SelectedItemIndex = 0;
                WindowSize = windowSize;
                WindowStart = 0;
                WindowEnd = windowSize - 1;
                TotalItems = newItems.size();
                StartBufferItem = 0;
                EndBufferItem = std::min(TotalItems, UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;
                ScrollRequired = 0;
            }

            void AddItems(const std::vector<const char *>& newItems){
                // allocate new memory for each new item and copy the string
                for(auto item : newItems){
                    items.push_back(strdup(item));
                }
                TotalItems = items.size();
                EndBufferItem = std::min(TotalItems, UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;
            }

            void AddItem(const char *newItem){
                items.push_back(strdup(newItem));
                TotalItems = items.size();
                EndBufferItem = std::min(TotalItems, UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;
            }

            // receive a formatted string, allocate memory and copy the string
            void AddItem(const __FlashStringHelper *format, ...){
                char buffer[UMD_DISPLAY_BUFFER_CHARS_PER_LINE+1];
                va_list args;
                va_start(args, format);
                vsnprintf_P(buffer, UMD_DISPLAY_BUFFER_CHARS_PER_LINE, (const char *)format, args);
                va_end(args);
                items.push_back(strdup(buffer));
                TotalItems = items.size();
                EndBufferItem = std::min(TotalItems, UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;
            }

            void ClearItems(){
                for(auto item : items){
                    free((void *)item);
                }
                items.clear();
            }
        }mWindow;

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
