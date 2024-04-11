
#include "umdDisplay.h"

Adafruit_SSD1306* UMDDisplay::_display = new Adafruit_SSD1306(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

UMDDisplay::UMDDisplay()
{
    this->clear();
}

bool UMDDisplay::begin()
{
    // I don't like being tied to Adafruit_SSD1306 like this
    //_display = new Adafruit_SSD1306(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);

    _cursor.character = '*';
    //place cursor offscreen;
    _cursor.x = 0;
    _cursor.y = 0;
    _cursor.visible = false;
    _menu.cursorVisible = false;

    _clock.framePointer = 0;
    _clock.x = 0;
    _clock.y = 0;
    _clock.visible = false;

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
    _cursor.character = c;
    _needsRedraw = true;
}

void UMDDisplay::setCursorVisible(bool visible)
{
    _cursor.visible = visible;
    _needsRedraw = true;
}

void UMDDisplay::setCursorPosition(int x, int y)
{
    if(x < OLED_MAX_CHARS_PER_LINE){
        _cursor.x = x * OLED_FONT_WIDTH;
    }else{
        _cursor.x = -1;
    }
    
    if(y < OLED_MAX_LINES_PER_SCREEN){
        _cursor.y = y * OLED_FONT_HEIGHT;
    }else{
        _cursor.y = -1;
    }
    _needsRedraw = true;
}

void UMDDisplay::setClockVisible(bool visible)
{
    _clock.visible = visible;
    _needsRedraw = true;
}

void UMDDisplay::setClockPosition(int x, int y)
{
    if(x < OLED_MAX_CHARS_PER_LINE){
        _clock.x = x * OLED_FONT_WIDTH;
    }else{
        _clock.x = -1;
    }
    
    if(y < OLED_MAX_LINES_PER_SCREEN){
        _clock.y = y * OLED_FONT_HEIGHT;
    }else{
        _clock.y = -1;
    }
    _needsRedraw = true;
}

void UMDDisplay::advanceClockAnimation()
{
    if(++_clock.framePointer == UMD_CLOCK_ANIMATION_FRAMES)
        _clock.framePointer = 0;
    _needsRedraw = true;
}

void UMDDisplay::setCursorMenuPosition()
{
    setCursorPosition(0, _menu.startLine + (_menu.currentItem - _menu.windowStart));
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

int UMDDisplay::showMenu(int layer, UMDMenuIndex menuIndex)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return -1;

    _menu.index = menuIndex;
    switch(menuIndex)
    {
        case UMD_MENU_MAIN:
            initMenu(layer, _mainMenu.Items, _mainMenu.Size);
            return _mainMenu.Size;
        case UMD_MENU_READ:
            initMenu(layer, _readMenu.Items, _readMenu.Size);
            return _readMenu.Size;
        case UMD_MENU_WRITE:
            initMenu(layer, _writeMenu.Items, _writeMenu.Size);
            return _writeMenu.Size;
        case UMD_MENU_TEST:
            initMenu(layer, _testMenu.Items, _testMenu.Size);
            return _testMenu.Size;
        default:
            clearLayer(layer);
            return 0;
    }
}

void UMDDisplay::showMenu(int layer, std::vector<const char *> items)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    // TODO there's surely an std way to copy a vector to another vector
    _menu.items.clear();
    for(int i = 0; i < items.size(); i++)
    {
        _menu.items.push_back(items[i]);
    }

    fillLayerFromMenu(layer, 0, 0);
    initMenuCursor(layer);
    _needsRedraw = true;
}

void UMDDisplay::initMenu(int layer, const char *menuItems[], int size)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    _menu.items.clear();
    for(int i = 0; i < size; i++)
    {
        _menu.items.push_back(menuItems[i]);
    }

    fillLayerFromMenu(layer, 0, 0);
    initMenuCursor(layer);
    _needsRedraw = true;
}

void UMDDisplay::initMenu(int layer, const __FlashStringHelper *menuItems[], int size)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    _menu.items.clear();
    for(int i = 0; i < size; i++)
    {
        _menu.items.push_back((const char *)menuItems[i]);
    }

    fillLayerFromMenu(layer, 0, 0);
    initMenuCursor(layer);
    _needsRedraw = true;
}

void UMDDisplay::menuCursorUpdate(int delta, bool visible)
{
    if(visible)
    {
        _menu.currentItem += delta;
        if(_menu.currentItem < 0)
        {
           _menu.currentItem = 0;
        }
        else if(_menu.currentItem >= _menu.items.size())
        {
            _menu.currentItem = _menu.items.size() - 1;
        }

        if(_menu.items.size() > _menu.windowSize)
        {
            // reached the top of the displayed menu and we need to scroll up?
            if(_menu.currentItem == _menu.windowStart && _menu.windowStart > 0)
            {
                _menu.scrollRequired = delta;
                _menu.windowStart += delta;
                _menu.windowEnd += delta;
                scrollMenu(delta);
            }
            // reached the bottom of the displayed menu and we need to scroll down?
            else if(_menu.currentItem == _menu.windowEnd && _menu.windowEnd < _menu.items.size())
            {
                _menu.scrollRequired = delta;
                _menu.windowStart += delta;
                _menu.windowEnd += delta;
                scrollMenu(delta);
            }

        }

        setCursorMenuPosition();
    }
    else
    {
        _menu.cursorVisible = false;
        setCursorPosition(-1, -1);
    }
}

int UMDDisplay::menuCurrentItem()
{
    return _menu.currentItem;
}

void UMDDisplay::initMenuCursor(int layer)
{
    _menu.cursorVisible = true;
    _menu.layer = layer;
    _menu.windowStart = 0;
    _menu.scrollRequired = 0;
    _menu.currentItem = 0;
    if(layer == 0)
    {
        _menu.startLine = 0;
        _menu.windowSize = std::min(_layerLength[layer], OLED_MAX_LINES_PER_SCREEN);
        
    }
    else
    {
        // find where this menu starts in display line space
        int line = 0;
        for(int prevLayers = 0; prevLayers < layer; prevLayers++)
        {
            line += _layerLength[prevLayers];
        }

        _menu.startLine = line;
        _menu.windowSize = std::min(_layerLength[layer], OLED_MAX_LINES_PER_SCREEN - line);
    }

    _menu.windowEnd = _menu.windowStart + _menu.windowSize - 1;
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
        if(menuIndex >= _menu.items.size())
        {
            //clearLine(bufferIndex);
            return;
        }
        else
        {
            // always leave 1 blank character at start of string for menus
            print(layer, _menu.items[menuIndex++], bufferIndex++, 1);
        }

        // wrap around the buffer
        if(bufferIndex >= UMD_DISPLAY_BUFFER_TOTAL_LINES)
            bufferIndex = 0;
    }
}

void UMDDisplay::scrollMenu(int delta)
{
    // easy case if menu is smaller than layer buffer
    if(_menu.items.size() < UMD_DISPLAY_BUFFER_TOTAL_LINES)
    {
        scrollY(_menu.layer, delta);
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
    if(_cursor.visible)
    {
        _display->setCursor(this->_cursor.x, this->_cursor.y);
        _display->print(this->_cursor.character);
    }

    // draw the clock if visible
    if(_clock.visible){
        _display->drawBitmap(_clock.x, _clock.y, _clockAnimation[_clock.framePointer], 8, 8, SSD1306_WHITE);
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