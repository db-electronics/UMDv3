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
#define UMD_DISPLAY_BUFFER_TOTAL_LINES      8

class UMDDisplay
{
    public:

        /// @brief display zones
        enum Zone : uint8_t
        {
            ZONE_TITLE = 0,
            ZONE_STATUS,
            ZONE_WINDOW
        };

        UMDDisplay(std::unique_ptr<Adafruit_SSD1306> display);

        /// @brief initialize the display
        bool Init();

        /// @brief set the visibility of a display zone
        /// @param zone the zone to set visibility for
        /// @param visible the visibility state
        void SetZoneVisibility(Zone zone, bool visible);

        /// @brief clear a zone of the display
        /// @param zone the zone to clear
        void ClearZone(Zone zone);

        /// @brief clear a line in a zone of the display
        void ClearLine(Zone zone, uint8_t lineNumber);

        /// @brief print a line directly to the output buffer, n.b. be wary of buffer line size
        /// @param zone the zone to print to 
        /// @param format the format string
        /// @param args the arguments to the format string
        void Printf(Zone zone, const __FlashStringHelper *format, ...);

        /// @brief allocate a new line in the window items and print to it
        /// @param format the format string
        /// @param args the arguments to the format string
        void Printf(const __FlashStringHelper *format, ...);

        /// @brief load a set of items of arbitrary length into the item buffer, destroys any existing items
        /// @param items the items to load
        void NewWindow(const std::vector<const char *>& items);

        /// @brief allocate a new line in the window items and add an item to it
        /// @param item the item to add
        void AddWindowItem(const char *item);

        /// @brief allocate new lines in the window items and add items to them
        /// @param items the items to add
        void AddWindowItems(const std::vector<const char *>& items);
        
        /// @brief destroy current window items
        void ClearWindowItems();

        /// @brief get the selected item index
        /// @return the selected item index
        int GetSelectedItemIndex() { return mWindow.SelectedItemIndex; }

        /// @brief update the cursor item position up or down, moves the display window as required
        /// @param delta displacement amount
        void UpdateCursorItemPosition(int delta);

        void SetCursorChar(char c);
        void SetCursorVisibility(bool visible);
        void SetCursorPosition(int x, int y);

        void ResetScrollX();
        void SetWindowItemScrollX(int delta);
        void SetWindowScrollY(int delta);

        void SetProgressBarVisibility(bool visible);
        void SetProgressBarSize(int width);
        void SetProgressBar(uint32_t progress, uint32_t max, bool showPercent = true);

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

        struct ProgressBar
        {
            int Width = 100;
            int Height = 8;
            bool Visible = false;
            uint32_t Progress = 0;
            uint32_t Max = 100;
            float Percent = 0.0f;
            bool ShowPercent = true;
        } mProgressBar;

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
        int Clamp(int value, int min, int max);

        struct WindowItemsData{
            std::vector<std::string> Items;
            int StartLine;          // display line where the window starts
            int SelectedItemIndex;  // index of the selected item in the display buffer
            int WindowSize;         // number of items that can be displayed in the window
            int WindowStart;        // index of the first item in the visible window
            int WindowEnd;          // index of the last item in the visible window    
            int StartBufferItem;    // index of the first item in the display buffer
            int EndBufferItem;      // index of the last item in the display buffer
            int ScrollRequired;

            void Reset(int startLine, int windowSize){
                Items.clear();      // destroys all string items too
                StartLine = startLine;
                SelectedItemIndex = 0;
                WindowSize = windowSize;
                WindowStart = 0;
                WindowEnd = windowSize - 1;
                StartBufferItem = 0;
                EndBufferItem = 0;
                ScrollRequired = 0;
            }

        } mWindow;

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
