
#include "services/UmdDisplay.h"

UMDDisplay::UMDDisplay(std::unique_ptr<Adafruit_SSD1306> display) 
    : mDisplay(std::move(display))
{

}

// MARK: Init()
bool UMDDisplay::Init(){

    if(!mDisplay->begin(SSD1306_SWITCHCAPVCC, 0x3c)) { 
        return false;
    }

    // clear all buffers
    ClearZone(Zone::ZONE_TITLE);
    ClearZone(Zone::ZONE_WINDOW);
    ClearZone(Zone::ZONE_STATUS);
    ResetScrollX();

    mTitleVisible = false;
    mWindowVisible = true;
    mStatusVisible = false;
    mWindowCurrentLine = 0;
    mWindowScrollY = 0;

    mDisplay->clearDisplay();
    mDisplay->setTextSize(1);             // Normal 1:1 pixel scale
    mFontInfo.Set(6,8);
    mDisplay->setTextColor(WHITE);        // Draw white text
    mDisplay->display();
    mRedrawScreen = false;

    return true;
}

// MARK: SetZoneVisibility()
void UMDDisplay::SetZoneVisibility(Zone zone, bool visible)
{
    mRedrawScreen = true;
    switch(zone)
    {
        case Zone::ZONE_TITLE:
            mTitleVisible = visible;
            break;
        case Zone::ZONE_WINDOW:
            mWindowVisible = visible;
            break;
        case Zone::ZONE_STATUS:
            mStatusVisible = visible;
            break;
        default:
            mRedrawScreen = false;
            break;
    }
}

// MARK: ClearZone()
void UMDDisplay::ClearZone(Zone zone)
{
    mRedrawScreen = true;
    switch(zone)
    {
        // single line zones
        case Zone::ZONE_STATUS:
        case Zone::ZONE_TITLE:
            ClearLine(zone, 0);
            break;
        // multi-line zones
        case Zone::ZONE_WINDOW:
            for(int i = 0; i < UMD_DISPLAY_BUFFER_TOTAL_LINES; i++)
            {
                ClearLine(zone, i);
            }
            break;
        default:
            mRedrawScreen = false;
            break;
    }
    
}

// MARK: ClearLine()
void UMDDisplay::ClearLine(Zone zone, uint8_t const lineNumber)
{
    switch(zone)
    {
        case Zone::ZONE_TITLE:
            std::fill(mTitleBuffer.begin(), mTitleBuffer.end(), ' ');
            mTitleBuffer[OLED_MAX_CHARS_PER_LINE] = '\0';
            break;
        case Zone::ZONE_STATUS:
            std::fill(mStatusBuffer.begin(), mStatusBuffer.end(), ' ');
            mStatusBuffer[OLED_MAX_CHARS_PER_LINE] = '\0';
            break;
        case Zone::ZONE_WINDOW:
            std::fill(mWindowBuffer[lineNumber].begin(), mWindowBuffer[lineNumber].end(), ' ');
            mWindowBuffer[lineNumber][UMD_DISPLAY_BUFFER_CHARS_PER_LINE] = '\0';
            break;
        default:
            mRedrawScreen = false;
            break;
    }

    mRedrawScreen = true;
}

// MARK: Printf()
void UMDDisplay::Printf(Zone zone, const __FlashStringHelper *format, ...){
    va_list args;
    va_start(args, format);
    mRedrawScreen = true;
    switch(zone)
    {
        case Zone::ZONE_TITLE:
            ClearZone(Zone::ZONE_TITLE);
            std::vsnprintf(mTitleBuffer.data(), mTitleBuffer.size(), (const char *)format, args);
            //vsnprintf(mTitleBuffer, OLED_MAX_CHARS_PER_LINE, (const char *)format, args);
            break;
        case Zone::ZONE_STATUS:
            ClearZone(Zone::ZONE_STATUS);
            std::vsnprintf(mStatusBuffer.data(), mStatusBuffer.size(), (const char *)format, args);
            break;
        case Zone::ZONE_WINDOW:
            ClearLine(Zone::ZONE_WINDOW, mWindowCurrentLine);
            std::vsnprintf(mWindowBuffer[mWindowCurrentLine].data(), mWindowBuffer[mWindowCurrentLine].size(), (const char *)format, args);
            mWindowCurrentLine++;
            mWindow.TotalItems++;
            if(mWindowCurrentLine >= UMD_DISPLAY_BUFFER_TOTAL_LINES)
            {
                mWindowCurrentLine = 0;
            }
            break;
        default:
            mRedrawScreen = false;
            break;
    }
    va_end(args);
}

// MARK: SetWindowItems()
void UMDDisplay::SetWindowItems(const std::vector<const char *>& items)
{
    int windowStartLine = mTitleVisible ? 1 : 0;

    mWindow.Reset(windowStartLine, GetWindowVisibleLinesCount(), items);
    mWindowCurrentLine = std::min(items.size(), (size_t)UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;
    LoadWindowItemsToBuffer();
    SetCursorPosition(0, windowStartLine);
}

int UMDDisplay::GetWindowVisibleLinesCount()
{
    int result = OLED_MAX_LINES_PER_SCREEN;
    if(mStatusVisible)
    {
        result--;
    }
    if(mTitleVisible)
    {
        result--;
    }
    return result;
}

// MARK: ResetScrollX()
void UMDDisplay::ResetScrollX()
{
    mRedrawScreen = true;
    std::fill(mWindowScrollX.begin(), mWindowScrollX.end(), 0);
}

void UMDDisplay::SetWindowItemScrollX(int delta)
{
    // don't scroll if not required to, will need the size of each string in the buffer
    int itemLength = strlen(mWindow.items[mWindow.SelectedItemIndex]);
    if(itemLength < OLED_MAX_CHARS_PER_LINE)
    {
        return;
    }

    mRedrawScreen = true;

    // find on which line the current item is
    int bufferStartIndex = (mWindowScrollY - mWindow.WindowStart) % UMD_DISPLAY_BUFFER_TOTAL_LINES;
    int bufferIndex = (mWindow.SelectedItemIndex + bufferStartIndex) % UMD_DISPLAY_BUFFER_TOTAL_LINES;
    // apply the scroll
    mWindowScrollX[bufferIndex] += delta;

    // limit range to prevent the text from wrapping around
    mWindowScrollX[bufferIndex] = std::clamp(mWindowScrollX[bufferIndex], 0, itemLength - OLED_MAX_CHARS_PER_LINE + 1);
}

// MARK: SetWindowScrollY()
void UMDDisplay::SetWindowScrollY(int delta)
{
    mRedrawScreen = true;
    mWindowScrollY += delta;
    // proper modulo wrap around for negative value
    mWindowScrollY = (mWindowScrollY + UMD_DISPLAY_BUFFER_TOTAL_LINES) % UMD_DISPLAY_BUFFER_TOTAL_LINES;
}

// MARK: SetCursorChar()
void UMDDisplay::SetCursorChar(char c)
{
    mCursor.character = c;
    mRedrawScreen = true;
}

void UMDDisplay::SetCursorVisibility(bool visible)
{
    mCursor.visible = visible;
    mRedrawScreen = true;
}

void UMDDisplay::SetCursorPosition(int x, int y)
{
    mCursor.x = x * mFontInfo.Width;
    mCursor.y = y * mFontInfo.Height;
    mRedrawScreen = true;
}

// MARK: UpdateCursorItemPosition()
void UMDDisplay::UpdateCursorItemPosition(int delta)
{
    uint8_t bufferIndex = 0;

    if(mCursor.visible)
    {
        mRedrawScreen = true;

        // clamp at ends
        mWindow.SelectedItemIndex += delta;
        mWindow.SelectedItemIndex = std::clamp(mWindow.SelectedItemIndex, 0, mWindow.TotalItems-1);

        // check if we need to adjust the window
        if(mWindow.TotalItems > mWindow.WindowSize)
        {
            // reached the top of the displayed menu and we need to scroll up?
            if(mWindow.SelectedItemIndex == mWindow.WindowStart && mWindow.WindowStart > 0)
            {
                // scroll up
                mWindow.ScrollRequired = delta;
                // adjust window
                mWindow.WindowStart += delta;
                mWindow.WindowEnd += delta;

                if(mWindow.SelectedItemIndex == mWindow.StartBufferItem)
                {
                    // are there more items before the buffer?
                    if(mWindow.StartBufferItem > 0)
                    {
                        // we're scrolling up, load new item at the top of the buffer
                        // adding modulo constant ensures modulo signs are correct, and doesn't affect the result of the modulo operation
                        bufferIndex = ((mWindowScrollY + UMD_DISPLAY_BUFFER_TOTAL_LINES)-1) % UMD_DISPLAY_BUFFER_TOTAL_LINES; 
                        LoadWindowItemToBuffer(mWindow.SelectedItemIndex - 1, bufferIndex);
                        // adjust buffer indexes
                        mWindow.StartBufferItem--;
                        mWindow.EndBufferItem--;
                    }
                }
            }
            // reached the bottom of the displayed menu and we need to scroll down?
            else if (mWindow.SelectedItemIndex == mWindow.WindowEnd && mWindow.WindowEnd < (mWindow.TotalItems-1))
            {
                // scroll down
                mWindow.ScrollRequired = delta;
                // adjust window
                mWindow.WindowStart += delta;
                mWindow.WindowEnd += delta;

                // do we need to load an additional item into the buffer?
                if(mWindow.SelectedItemIndex == mWindow.EndBufferItem)
                {
                    // are there more items passed the buffer?
                    if(mWindow.EndBufferItem < (mWindow.TotalItems-1))
                    {
                        bufferIndex = (mWindowScrollY + mWindow.WindowSize) % UMD_DISPLAY_BUFFER_TOTAL_LINES;
                        LoadWindowItemToBuffer(mWindow.SelectedItemIndex + 1, bufferIndex);
                        // adjust buffer indexes
                        mWindow.StartBufferItem++;
                        mWindow.EndBufferItem++;
                    }
                }
            }
        }

        if(mWindow.ScrollRequired != 0)
        {
            SetWindowScrollY(mWindow.ScrollRequired);
            mWindow.ScrollRequired = 0;
        }

        SetCursorPosition(0, mWindow.StartLine + (mWindow.SelectedItemIndex - mWindow.WindowStart)); 

    }else{
        mRedrawScreen = false;
    }
}

uint8_t UMDDisplay::GetSelectedItemIndex()
{
    return (uint8_t)mWindow.SelectedItemIndex;
}

// MARK: LoadWindowItemsToBuffer()
void UMDDisplay::LoadWindowItemsToBuffer()
{

    int itemChar = 0;
    int bufferChar = 0;

    // clear the window buffer
    ClearZone(Zone::ZONE_WINDOW);
    for(int i = 0; i < UMD_DISPLAY_BUFFER_TOTAL_LINES; i++)
    {
        // have we reached the end of the menu items?

        if(i >= mWindow.items.size())
        {
            return;
        }
        else
        {
            // always leave 1 blank character at start of string for cursor (already blanked from ClearZone)
            // strncpy fills with null characters, not useful for scrolling
            //strncpy(mWindowBuffer[i].data()+1, mWindow.items[i], UMD_DISPLAY_BUFFER_CHARS_PER_LINE-1);
            strcpy(mWindowBuffer[i].data()+1, mWindow.items[i]);
            // itemChar = 0;
            // bufferChar = 0;
            // while(mWindow.items[i][itemChar] != '\0')
            // {
            //     bufferChar = (bufferChar + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
            //     mWindowBuffer[i][bufferChar] = mWindow.items[i][itemChar];
            //     itemChar = (itemChar + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
            // }
        }
    }
    mRedrawScreen = true;
}

void UMDDisplay::LoadWindowItemToBuffer(int itemIndex, int bufferIndex)
{
    uint8_t itemChar = 0;
    uint8_t bufferChar = 0;

    if(itemIndex >= mWindow.items.size())
    {
        return;
    }
    else
    {
        ClearLine(Zone::ZONE_WINDOW, bufferIndex);
        //strncpy(mWindowBuffer[bufferIndex].data()+1, mWindow.items[itemIndex], UMD_DISPLAY_BUFFER_CHARS_PER_LINE-1);
        // always leave 1 blank character at start of string for cursor (already blanked from ClearZone)
        strcpy(mWindowBuffer[bufferIndex].data()+1, mWindow.items[itemIndex]);
        // itemChar = 0;
        // bufferChar = 0;
        // while(mWindow.items[itemIndex][itemChar] != '\0')
        // {
        //     bufferChar = (bufferChar + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
        //     mWindowBuffer[bufferIndex][bufferChar] = mWindow.items[itemIndex][itemChar];
        //     itemChar = (itemChar + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
        // }
    }
    mRedrawScreen = true;
}

// MARK: Redraw()
void UMDDisplay::Redraw(void){
    uint8_t currentLineOnDisplay = 0;
    uint8_t windowLinesToPrint = 0;
    uint8_t windowCharPos = 0;
    uint8_t windowLineIndex = mWindowScrollY;

    if(!mRedrawScreen)
        return;

    auto linePosToCoordinate = [h = this->mFontInfo.Height](uint8_t lineNumber) {return lineNumber * h;};

    mDisplay->clearDisplay();

    // Title on line 0, if visible
    if(mTitleVisible)
    {
        mDisplay->setCursor(0, linePosToCoordinate(currentLineOnDisplay));
        mDisplay->print(mTitleBuffer.data());
        currentLineOnDisplay++;
    }

    // Window on lines 1-n, if visible
    if(mWindowVisible)
    {
        // determine the size of the window to print
        windowLinesToPrint = GetWindowVisibleLinesCount();

        for(int i = 0 ; i < windowLinesToPrint; i++)
        {
            mDisplay->setCursor(0, linePosToCoordinate(currentLineOnDisplay));
            windowCharPos = mWindowScrollX[(windowLineIndex) % UMD_DISPLAY_BUFFER_TOTAL_LINES];
            for(uint8_t dispPosition = 0; dispPosition < OLED_MAX_CHARS_PER_LINE; dispPosition++)
            {
                // copy the character from the window buffer to the line buffer
                mLineBuffer[dispPosition] = mWindowBuffer[windowLineIndex][windowCharPos];
                // wrap around character pointer
                windowCharPos = (windowCharPos + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
                
            }
            mLineBuffer[OLED_MAX_CHARS_PER_LINE] = '\0'; // Ensure null-termination
            mDisplay->print(mLineBuffer.data());
            windowLineIndex = (windowLineIndex + 1) % UMD_DISPLAY_BUFFER_TOTAL_LINES;
            currentLineOnDisplay++;
        }
    }

    // Status on the final line, if visible
    if(mStatusVisible)
    {
        mDisplay->setCursor(0, linePosToCoordinate(currentLineOnDisplay));
        mDisplay->print(mStatusBuffer.data());
    }

    // draw the cursor if visible
    if(mCursor.visible)
    {
        mDisplay->setCursor(this->mCursor.x, this->mCursor.y);
        mDisplay->print(this->mCursor.character);
    }

    mDisplay->display();
    mRedrawScreen = false;
}
