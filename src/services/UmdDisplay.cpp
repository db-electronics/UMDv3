
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
    mProgressBar.Visible = false;
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

void UMDDisplay::SetProgressBarVisibility(bool visible)
{
    mRedrawScreen = true;
    mProgressBar.Visible = visible;
}

void UMDDisplay::SetProgressBarSize(int width)
{
    mRedrawScreen = true;
    mProgressBar.Width = std::max(width, 100);
}

void UMDDisplay::SetProgressBar(uint32_t progress, uint32_t max, bool showPercent)
{
    mRedrawScreen = true;
    mProgressBar.Progress = progress;
    mProgressBar.Max = std::min(max, (uint32_t)1); // prevent division by zero
    mProgressBar.Percent = (float)progress / (float)max;
    mProgressBar.ShowPercent = showPercent;
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
            break;
        case Zone::ZONE_STATUS:
            ClearZone(Zone::ZONE_STATUS);
            std::vsnprintf(mStatusBuffer.data(), mStatusBuffer.size(), (const char *)format, args);
            break;
        case Zone::ZONE_WINDOW:
            ClearLine(Zone::ZONE_WINDOW, mWindowCurrentLine);
            std::vsnprintf(mWindowBuffer[mWindowCurrentLine].data(), mWindowBuffer[mWindowCurrentLine].size(), (const char *)format, args);
            mWindowCurrentLine = (mWindowCurrentLine + 1) % UMD_DISPLAY_BUFFER_TOTAL_LINES;
            break;
        default:
            mRedrawScreen = false;
            break;
    }
    va_end(args);
}

// MARK: NewWindowItems()
void UMDDisplay::NewWindow(const std::vector<const char *>& items)
{
    int windowStartLine = mTitleVisible ? 1 : 0;
    mWindowScrollY = 0;
    ResetScrollX();
    mWindow.Reset(windowStartLine, GetWindowVisibleLinesCount());
    AddWindowItems(items);
    LoadWindowItemsToBuffer();
    SetCursorPosition(0, mWindow.StartLine);
}

void UMDDisplay::AddWindowItem(const char *item)
{
    mWindow.Items.push_back(std::string(item));
    mWindow.EndBufferItem = std::min((int)mWindow.Items.size(), UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;
}

void UMDDisplay::Printf(const __FlashStringHelper *format, ...){
    va_list args;
    va_start(args, format);

    // Get the length of the formatted string
    va_list args_copy;
    va_copy(args_copy, args);
    int len = std::vsnprintf(nullptr, 0, (const char *)format, args_copy);
    va_end(args_copy);

    // Allocate an std::string of the appropriate size
    std::string str(len, ' ');

    // Print the formatted string to the std::string
    std::vsprintf(&str[0], (const char *)format, args);

    va_end(args);

    mWindow.Items.push_back(str);
    mWindow.EndBufferItem = std::min((int)mWindow.Items.size(), UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;

    // just reload the buffer, it's easier
    LoadWindowItemsToBuffer();
}

void UMDDisplay::AddWindowItems(const std::vector<const char *>& items)
{
    for(auto item : items){
        mWindow.Items.push_back(std::string(item));
    }
    mWindow.EndBufferItem = std::min((int)mWindow.Items.size(), UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;
}

void UMDDisplay::ClearWindowItems()
{
    mWindow.Items.clear();
    ClearZone(Zone::ZONE_WINDOW);
}

int UMDDisplay::GetWindowVisibleLinesCount()
{
    int result = OLED_MAX_LINES_PER_SCREEN;
    if(mStatusVisible || mProgressBar.Visible)
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
    // don't scroll if not required to
    int itemLength = mWindow.Items[mWindow.SelectedItemIndex].length();
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
    mWindowScrollX[bufferIndex] = Clamp(mWindowScrollX[bufferIndex], 0, itemLength - OLED_MAX_CHARS_PER_LINE + 1);
}

int UMDDisplay::Clamp(int value, int min, int max)
{
    if(value < min)
    {
        return min;
    }
    else if (value > max)
    {
        return max;
    }
    else
    {
        return value;
    }
    
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
        mWindow.SelectedItemIndex = Clamp(mWindow.SelectedItemIndex, 0, (int)mWindow.Items.size()-1);

        // check if we need to adjust the window
        if(mWindow.Items.size() > mWindow.WindowSize)
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
            else if (mWindow.SelectedItemIndex == mWindow.WindowEnd && mWindow.WindowEnd < (mWindow.Items.size()-1))
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
                    if(mWindow.EndBufferItem < (mWindow.Items.size()-1))
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

        if(i >= mWindow.Items.size())
        {
            return;
        }
        else
        {
            // always leave 1 blank character at start of string for cursor (already blanked from ClearZone)
            strcpy(mWindowBuffer[i].data()+1, mWindow.Items[i].c_str());
        }
    }
    mRedrawScreen = true;
}

void UMDDisplay::LoadWindowItemToBuffer(int itemIndex, int bufferIndex)
{
    uint8_t itemChar = 0;
    uint8_t bufferChar = 0;

    if(itemIndex >= mWindow.Items.size())
    {
        return;
    }
    else
    {
        ClearLine(Zone::ZONE_WINDOW, bufferIndex);
        // always leave 1 blank character at start of string for cursor (already blanked from ClearZone)
        strcpy(mWindowBuffer[bufferIndex].data()+1, mWindow.Items[itemIndex].c_str());
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
        //mDisplay->setTextColor(BLACK, WHITE);
        mDisplay->print(mTitleBuffer.data());
        //mDisplay->setTextColor(WHITE, BLACK);
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

    // Progress bar, if visible will override the status line
    if(mProgressBar.Visible)
    {
        // draw the progress bar padded by 2 pixels, fill represents the percent complete
        mDisplay->drawRect(2, linePosToCoordinate(currentLineOnDisplay), mProgressBar.Width, mProgressBar.Height, WHITE);
        mDisplay->fillRect(2, linePosToCoordinate(currentLineOnDisplay), mProgressBar.Width * mProgressBar.Percent, mProgressBar.Height, WHITE);
        
        if(mProgressBar.ShowPercent)
        {
            mDisplay->setCursor(mProgressBar.Width + 4, linePosToCoordinate(currentLineOnDisplay));
            mDisplay->print((int)(mProgressBar.Percent * 100));
            mDisplay->print('%');
        }
        currentLineOnDisplay++;
    }
    else if(mStatusVisible)
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
