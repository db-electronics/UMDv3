
#include "services/UmdDisplay.h"

Adafruit_SSD1306* UMDDisplay::_display = new Adafruit_SSD1306(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

UMDDisplay::UMDDisplay()
{

}

// MARK: Init()
bool UMDDisplay::Init(){

    if(!_display->begin(SSD1306_SWITCHCAPVCC, 0x3c)) { 
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

    _display->clearDisplay();
    _display->setTextSize(1);             // Normal 1:1 pixel scale
    mFontInfo.Set(6,8);
    _display->setTextColor(WHITE);        // Draw white text
    _display->display();
    mRedrawScreen = false;

    return true;
}

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
    uint8_t windowStartLine = mTitleVisible ? 1 : 0;

    mWindow.Reset(windowStartLine, GetWindowVisibleLinesCount(), items);
    mWindowCurrentLine = std::min(items.size(), (size_t)UMD_DISPLAY_BUFFER_TOTAL_LINES) - 1;
    LoadWindowItemsToBuffer();
    SetCursorPosition(0, windowStartLine);
}

uint8_t UMDDisplay::GetWindowVisibleLinesCount()
{
    uint8_t result = OLED_MAX_LINES_PER_SCREEN;
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

void UMDDisplay::IncWindowScrollX(uint8_t lineNumber)
{
    uint8_t scroll;

    mRedrawScreen = true;
    if(++mWindowScrollX[lineNumber] >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE){
        mWindowScrollX[lineNumber] = 0;
    }
}

void UMDDisplay::DecWindowScrollX(uint8_t lineNumber)
{
    mRedrawScreen = true;
    if(mWindowScrollX[lineNumber] == 0){
        mWindowScrollX[lineNumber] = UMD_DISPLAY_BUFFER_TOTAL_LINES - 1;
    }else{
        mWindowScrollX[lineNumber]--;
    }
}

// MARK: SetWindowScrollY()
void UMDDisplay::SetWindowScrollY(int8_t delta)
{
    mRedrawScreen = true;
    mWindowScrollY += delta;
    // limit range to the buffer line size

    if(mWindowScrollY < 0){
        mWindowScrollY = UMD_DISPLAY_BUFFER_TOTAL_LINES - 1;
    }else if(mWindowScrollY >= UMD_DISPLAY_BUFFER_TOTAL_LINES){
        mWindowScrollY = 0;
    }
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

void UMDDisplay::ClearScratchBufferLine(int lineNumber)
{
    for(int i = 0; i < UMD_DISPLAY_BUFFER_CHARS_PER_LINE; i++)
    {
        ScratchBuffer[lineNumber][i] = ' ';
    }
}

void UMDDisplay::printf(int layer, int lineNumber, const char *format, ...)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    va_list args;
    va_start(args, format);
    vsnprintf(_buffer[layer][lineNumber], UMD_DISPLAY_BUFFER_CHARS_PER_LINE, format, args);
    va_end(args);
    _needsRedraw = true;
}



void UMDDisplay::printf(int layer, int lineNumber, const __FlashStringHelper *format, ...)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;
    
    va_list args;
    va_start(args, format);
    vsnprintf(_buffer[layer][lineNumber], UMD_DISPLAY_BUFFER_CHARS_PER_LINE, (const char *)format, args);
    va_end(args);
    _needsRedraw = true;
}

void UMDDisplay::print(int layer, const char characters[], int lineNumber, int printPos)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;
    
    int i = 0;
    _bufferNextPos[layer][lineNumber] = printPos;

    if(_bufferNextPos[layer][lineNumber] >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE){
        _bufferNextPos[layer][lineNumber] = 0;
    }

    while(characters[i] != '\0'){
        // wrap around if required
        if(i >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE )
        {
            _bufferNextPos[layer][lineNumber] = 0;
        }
        
        //this->buffer[lineNumber][bufferNextPos[lineNumber]++] = characters[i++];
        _buffer[layer][lineNumber][_bufferNextPos[layer][lineNumber]++] = characters[i++];
    }
}

void UMDDisplay::print(int layer, const char characters[], int lineNumber)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    print(layer, characters, lineNumber, _bufferNextPos[layer][lineNumber]);
}

void UMDDisplay::print(int layer, int number, int lineNumber)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    std::string tmp = std::to_string(number);
    const char *num_char = tmp.c_str();
    print(layer, num_char, lineNumber);
}


// MARK: UpdateCursorItemPosition()
void UMDDisplay::UpdateCursorItemPosition(int8_t delta)
{
    uint8_t bufferIndex = 0;

    if(mCursor.visible)
    {
        mRedrawScreen = true;

        // cap at ends of list
        mWindow.SelectedItemIndex += delta;
        if(mWindow.SelectedItemIndex < 0)
        {
            mWindow.SelectedItemIndex = 0;
        }
        else if(mWindow.SelectedItemIndex >= mWindow.TotalItems)
        {
            mWindow.SelectedItemIndex = mWindow.TotalItems - 1;
        }

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
    // let's be safe
    uint8_t charIndex = 0;

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
            strncpy(mWindowBuffer[i].data()+1, mWindow.items[i], UMD_DISPLAY_BUFFER_CHARS_PER_LINE-1);
            charIndex = 0;
            // while(mWindow.items[i][charIndex] != '\0'){
            //     mWindowBuffer[i][charIndex+1] = mWindow.items[i][charIndex];
            //     charIndex = (charIndex + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
            // }
        }
    }
    mRedrawScreen = true;
}

void UMDDisplay::LoadWindowItemToBuffer(uint8_t itemIndex, uint8_t bufferIndex)
{
    uint8_t charIndex = 0;
    if(itemIndex >= mWindow.items.size())
    {
        return;
    }
    else
    {
        ClearLine(Zone::ZONE_WINDOW, bufferIndex);
        strncpy(mWindowBuffer[bufferIndex].data()+1, mWindow.items[itemIndex], UMD_DISPLAY_BUFFER_CHARS_PER_LINE-1);
        // always leave 1 blank character at start of string for cursor (already blanked from ClearZone)
        // while(mWindow.items[itemIndex][charIndex] != '\0'){
        //     mWindowBuffer[bufferIndex][(charIndex + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE] = mWindow.items[itemIndex][charIndex];
        //     charIndex = (charIndex + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
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

    _display->clearDisplay();

    // Title on line 0, if visible
    if(mTitleVisible)
    {
        _display->setCursor(0, linePosToCoordinate(currentLineOnDisplay));
        _display->print(mTitleBuffer.data());
        currentLineOnDisplay++;
    }

    // Window on lines 1-n, if visible
    if(mWindowVisible)
    {
        // determine the size of the window to print
        windowLinesToPrint = GetWindowVisibleLinesCount();

        for(int i = 0 ; i < windowLinesToPrint; i++)
        {
            _display->setCursor(0, linePosToCoordinate(currentLineOnDisplay));
            windowCharPos = mWindowScrollX[(windowLineIndex) % UMD_DISPLAY_BUFFER_TOTAL_LINES];
            for(uint8_t dispPosition = 0; dispPosition < OLED_MAX_CHARS_PER_LINE; dispPosition++)
            {
                // copy the character from the window buffer to the line buffer
                mLineBuffer[dispPosition] = mWindowBuffer[windowLineIndex][windowCharPos];
                // wrap around character pointer
                windowCharPos = (windowCharPos + 1) % UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
                
            }
            mLineBuffer[OLED_MAX_CHARS_PER_LINE] = '\0'; // Ensure null-termination
            _display->print(mLineBuffer.data());
            windowLineIndex = (windowLineIndex + 1) % UMD_DISPLAY_BUFFER_TOTAL_LINES;
            currentLineOnDisplay++;
        }
    }

    // Status on the final line, if visible
    if(mStatusVisible)
    {
        _display->setCursor(0, linePosToCoordinate(currentLineOnDisplay));
        _display->print(mStatusBuffer.data());
    }

    // draw the cursor if visible
    if(mCursor.visible)
    {
        _display->setCursor(this->mCursor.x, this->mCursor.y);
        _display->print(this->mCursor.character);
    }

    _display->display();
    mRedrawScreen = false;
}

void UMDDisplay::scrollX(int layer, int lineNumber, int delta)
{
    if(delta > UMD_DISPLAY_BUFFER_CHARS_PER_LINE)
        return;

    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    if(lineNumber >= OLED_MAX_LINES_PER_SCREEN)
        return;

    _scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_CHAR] += delta;
    if(_scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_CHAR] >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE)
    {
        _scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_CHAR] -= UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
    }
    else if(_scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_CHAR] < 0)
    {
        _scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_CHAR] += UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
    }
    _needsRedraw = true;
}

void UMDDisplay::scrollY(int layer, int delta)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    if(delta > UMD_DISPLAY_BUFFER_TOTAL_LINES)
        return;

    for(int lineNumber = 0; lineNumber < OLED_MAX_LINES_PER_SCREEN; lineNumber++)
    {
        _scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_LINE] += delta;
        if(_scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_LINE] >= UMD_DISPLAY_BUFFER_TOTAL_LINES)
        {
            _scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_LINE] = 0;
        }
        else if(_scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_LINE] < 0)
        {
            _scroll[layer][lineNumber][0] = UMD_DISPLAY_BUFFER_TOTAL_LINES-1;
        }
    }
    _needsRedraw = true;
}

