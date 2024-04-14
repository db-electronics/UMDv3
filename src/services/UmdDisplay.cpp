
#include "services/UmdDisplay.h"

Adafruit_SSD1306* UMDDisplay::_display = new Adafruit_SSD1306(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

UMDDisplay::UMDDisplay()
{
    this->clear();
}

bool UMDDisplay::begin()
{
    // I don't like being tied to Adafruit_SSD1306 like this
    //_display = new Adafruit_SSD1306(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

    mCursor.character = '*';
    //place cursor offscreen;
    mCursor.x = 0;
    mCursor.y = 0;
    mCursor.visible = false;
    mMenu.cursorVisible = false;

    mClockSpr.framePointer = 0;
    mClockSpr.x = 0;
    mClockSpr.y = 0;
    mClockSpr.visible = false;

    if(!_display->begin(SSD1306_SWITCHCAPVCC, 0x3c)) { 
        return false;
    }

    int layerLength = UMD_DISPLAY_BUFFER_TOTAL_LINES;
    for(int layer = 0; layer < UMD_DISPLAY_LAYERS; layer++)
    {
        for(int i = 0; i < OLED_MAX_LINES_PER_SCREEN; i++)
        {
            // scroll[i][0] = i;
            // scroll[i][1] = 0;
            _scroll[layer][i][UMD_DISPLAY_SCROLL_LINE] = i;
        }
        
        // only enable first layer on init
        _layerLength[layer] = layerLength;
        layerLength = 0;
    }

    _display->display();
    //delay(2000); // Show splash screen for 2 seconds
    _display->clearDisplay();
    _display->setTextSize(1);             // Normal 1:1 pixel scale
    _display->setTextColor(WHITE);        // Draw white text
    _display->display();
    _needsRedraw = false;
    return true;
}

void UMDDisplay::setCursorChar(char c)
{
    mCursor.character = c;
    _needsRedraw = true;
}

void UMDDisplay::setCursorVisible(bool visible)
{
    mCursor.visible = visible;
    _needsRedraw = true;
}

void UMDDisplay::setCursorPosition(int x, int y)
{
    if(x < OLED_MAX_CHARS_PER_LINE){
        mCursor.x = x * OLED_FONT_WIDTH;
    }else{
        mCursor.x = -1;
    }
    
    if(y < OLED_MAX_LINES_PER_SCREEN){
        mCursor.y = y * OLED_FONT_HEIGHT;
    }else{
        mCursor.y = -1;
    }
    _needsRedraw = true;
}

void UMDDisplay::setClockVisible(bool visible)
{
    mClockSpr.visible = visible;
    _needsRedraw = true;
}

void UMDDisplay::setClockPosition(int x, int y)
{
    if(x < OLED_MAX_CHARS_PER_LINE){
        mClockSpr.x = x * OLED_FONT_WIDTH;
    }else{
        mClockSpr.x = -1;
    }
    
    if(y < OLED_MAX_LINES_PER_SCREEN){
        mClockSpr.y = y * OLED_FONT_HEIGHT;
    }else{
        mClockSpr.y = -1;
    }
    _needsRedraw = true;
}

void UMDDisplay::advanceClockAnimation()
{
    if(!mClockSpr.visible)
        return;
        
    if(++mClockSpr.framePointer == UMD_CLOCK_ANIMATION_FRAMES)
        mClockSpr.framePointer = 0;
    _needsRedraw = true;
}

void UMDDisplay::setCursorMenuPosition()
{
    setCursorPosition(0, mMenu.startLine + (mMenu.currentItem - mMenu.windowStart));
}

void UMDDisplay::setLayerLineLength(int layer, int length)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    if(length > UMD_DISPLAY_BUFFER_TOTAL_LINES)
        length = UMD_DISPLAY_BUFFER_TOTAL_LINES;

    _layerLength[layer] = length;
}

void UMDDisplay::clear(void)
{
    for(int layer = 0; layer < UMD_DISPLAY_LAYERS; layer++)
    {
        // for(int line = 0; line < UMD_DISPLAY_BUFFER_TOTAL_LINES; line++)
        // {
        //     clearLine(layer, line);
        // }
        clearLayer(layer);
    }

    _needsRedraw = true;
}


void UMDDisplay::clearLayer(int layer)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    for(int line = 0; line < UMD_DISPLAY_BUFFER_TOTAL_LINES; line++)
    {
        clearLine(layer, line);
    }
    _needsRedraw = true;
}

void UMDDisplay::clearLine(int layer, int lineNumber)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    for(int chr = 0; chr < UMD_DISPLAY_BUFFER_CHARS_PER_LINE; chr++)
    {
        _buffer[layer][lineNumber][chr] = ' ';
    }
    _needsRedraw = true;
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

void UMDDisplay::LoadMenuItems(int layer, std::vector<const char *>& items)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    mMenu.items.assign(items.begin(), items.end());

    fillLayerFromMenu(layer, 0, 0);
    initMenuCursor(layer);
    _needsRedraw = true;
}

void UMDDisplay::menuCursorUpdate(int delta, bool visible)
{
    if(visible)
    {
        mMenu.currentItem += delta;
        if(mMenu.currentItem < 0)
        {
           mMenu.currentItem = 0;
        }
        else if(mMenu.currentItem >= mMenu.items.size())
        {
            mMenu.currentItem = mMenu.items.size() - 1;
        }

        if(mMenu.items.size() > mMenu.windowSize)
        {
            // reached the top of the displayed menu and we need to scroll up?
            if(mMenu.currentItem == mMenu.windowStart && mMenu.windowStart > 0)
            {
                mMenu.scrollRequired = delta;
                mMenu.windowStart += delta;
                mMenu.windowEnd += delta;
                scrollMenu(delta);
            }
            // reached the bottom of the displayed menu and we need to scroll down?
            else if(mMenu.currentItem == mMenu.windowEnd && mMenu.windowEnd < mMenu.items.size())
            {
                mMenu.scrollRequired = delta;
                mMenu.windowStart += delta;
                mMenu.windowEnd += delta;
                scrollMenu(delta);
            }

        }

        setCursorMenuPosition();
    }
    else
    {
        mMenu.cursorVisible = false;
        setCursorPosition(-1, -1);
    }
}

uint8_t UMDDisplay::GetCurrentItemIndex()
{
    return mMenu.currentItem;
}

void UMDDisplay::initMenuCursor(int layer)
{
    mMenu.cursorVisible = true;
    mMenu.layer = layer;
    mMenu.windowStart = 0;
    mMenu.scrollRequired = 0;
    mMenu.currentItem = 0;
    if(layer == 0)
    {
        mMenu.startLine = 0;
        mMenu.windowSize = std::min(_layerLength[layer], OLED_MAX_LINES_PER_SCREEN);
        
    }
    else
    {
        // find where this menu starts in display line space
        int line = 0;
        for(int prevLayers = 0; prevLayers < layer; prevLayers++)
        {
            line += _layerLength[prevLayers];
        }

        mMenu.startLine = line;
        mMenu.windowSize = std::min(_layerLength[layer], OLED_MAX_LINES_PER_SCREEN - line);
    }

    mMenu.windowEnd = mMenu.windowStart + mMenu.windowSize - 1;
    setCursorMenuPosition();
}

void UMDDisplay::fillLayerFromMenu(int layer, int startBufferIndex, int startMenuIndex)
{
    // clear the layer
    clearLayer(layer);

    // fill the buffer with menu items
    int menuIndex = startMenuIndex;
    int bufferIndex = startBufferIndex;

    for(int i = 0; i < UMD_DISPLAY_BUFFER_TOTAL_LINES; i++)
    {
        // have we reached the end of the menu items? if so clear the rest of the buffer
        if(menuIndex >= mMenu.items.size())
        {
            //clearLine(bufferIndex);
            return;
        }
        else
        {
            // always leave 1 blank character at start of string for menus
            print(layer, mMenu.items[menuIndex++], bufferIndex++, 1);
        }

        // wrap around the buffer
        if(bufferIndex >= UMD_DISPLAY_BUFFER_TOTAL_LINES)
            bufferIndex = 0;
    }
}

void UMDDisplay::scrollMenu(int delta)
{
    // easy case if menu is smaller than layer buffer
    if(mMenu.items.size() < UMD_DISPLAY_BUFFER_TOTAL_LINES)
    {
        scrollY(mMenu.layer, delta);
    }
    else
    {

    }
}

void UMDDisplay::redraw(void)
{
    if(!_needsRedraw)
        return;
    
    char lineChars[OLED_MAX_CHARS_PER_LINE+1]; // +1 for terminator
    int charIndexFromBuffer, lineIndexFromBuffer;

    _display->clearDisplay();

    int totalLinesDrawn = 0;
    for(int layer = 0; layer < UMD_DISPLAY_LAYERS; layer++)
    {
        // draw all lines from layer up to OLED_MAX_LINES_PER_SCREEN
        int layerLength = _layerLength[layer];
        if( (layerLength + totalLinesDrawn) > OLED_MAX_LINES_PER_SCREEN)
            layerLength = OLED_MAX_LINES_PER_SCREEN - totalLinesDrawn;

        for(int line = 0; line < layerLength; line++)
        {
            _display->setCursor(0, OLED_LINE_NUMBER(totalLinesDrawn));
            lineIndexFromBuffer = _scroll[layer][line][UMD_DISPLAY_SCROLL_LINE];
            charIndexFromBuffer = _scroll[layer][lineIndexFromBuffer][UMD_DISPLAY_SCROLL_CHAR];
            
            // file the lineChars buffer
            for(int pos = 0; pos < OLED_MAX_CHARS_PER_LINE; pos++)
            {
                // wrap around if required
                if(charIndexFromBuffer >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE )
                {
                    charIndexFromBuffer = 0;
                }
                lineChars[pos] = _buffer[layer][lineIndexFromBuffer][charIndexFromBuffer++];
            }
            lineChars[OLED_MAX_CHARS_PER_LINE] = '\0'; // Ensure null-termination
            totalLinesDrawn++;
            _display->print(lineChars);
        }
    }

    // draw the cursor if visible
    if(mCursor.visible)
    {
        _display->setCursor(this->mCursor.x, this->mCursor.y);
        _display->print(this->mCursor.character);
    }

    // draw the clock if visible
    if(mClockSpr.visible){
        _display->drawBitmap(mClockSpr.x, mClockSpr.y, _clockAnimation[mClockSpr.framePointer], 8, 8, SSD1306_WHITE);
    }

    _display->display();
    // clear the next position trackers
    for(int layer = 0; layer < UMD_DISPLAY_LAYERS; layer++)
    {
        for(int i = 0; i < UMD_DISPLAY_BUFFER_TOTAL_LINES; i++)
        {
            _bufferNextPos[layer][i] = 0;
        }
    }
    
    _needsRedraw = false;
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

void UMDDisplay::ResetScrollX(int layer)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    for(int lineNumber = 0; lineNumber < OLED_MAX_LINES_PER_SCREEN; lineNumber++)
    {
        _scroll[layer][lineNumber][UMD_DISPLAY_SCROLL_CHAR] = 0;
    }
    _needsRedraw = true;
}