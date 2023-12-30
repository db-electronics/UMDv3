
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

    _cursorChar = '*';
    //place cursor offscreen;
    _cursorPosition.x = -1;
    _cursorPosition.y = -1;
    _menuCursor.active = false;

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
    delay(2000); // Show splash screen for 2 seconds
    _display->clearDisplay();
    _display->setTextSize(1);             // Normal 1:1 pixel scale
    _display->setTextColor(WHITE);        // Draw white text
    _display->display();
    _needsRedraw = false;
    return true;
}

void UMDDisplay::setCursorChar(char c)
{
    _cursorChar = c;
    _needsRedraw = true;
}

void UMDDisplay::setCursorPosition(int x, int y)
{
    if(x < OLED_MAX_CHARS_PER_LINE){
        _cursorPosition.x = x * OLED_FONT_WIDTH;
    }else{
        _cursorPosition.x = -1;
    }
    
    if(y < OLED_MAX_LINES_PER_SCREEN){
        _cursorPosition.y = y * OLED_FONT_HEIGHT;
    }else{
        _cursorPosition.y = -1;
    }
    _needsRedraw = true;
}

void UMDDisplay::setCursorMenuPosition()
{
    setCursorPosition(0, _menuCursor.startLine + _menuCursor.item);
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
        for(int line = 0; line < UMD_DISPLAY_BUFFER_TOTAL_LINES; line++)
        {
            clearLine(layer, line);
        }
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

void UMDDisplay::initMenu(int layer, const char *menuItems[], int size)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    _menu.clear();
    for(int i = 0; i < size; i++)
    {
        _menu.push_back(menuItems[i]);
    }

    fillLayerFromMenu(layer, 0, 0);
    initMenuCursor(layer, size);
    _needsRedraw = true;
}

void UMDDisplay::initMenu(int layer, const __FlashStringHelper *menuItems[], int size)
{
    if(layer >= UMD_DISPLAY_LAYERS)
        return;

    _menu.clear();
    for(int i = 0; i < size; i++)
    {
        _menu.push_back((const char *)menuItems[i]);
    }

    fillLayerFromMenu(layer, 0, 0);
    initMenuCursor(layer, size);
    _needsRedraw = true;
}

void UMDDisplay::menuCursorUpdate(int delta, bool active)
{
    if(active)
    {
        delta %= _menuCursor.size;
        _menuCursor.item += delta;
        if(_menuCursor.item < 0)
        {
            _menuCursor.item += _menuCursor.size;
        }
        else if(_menuCursor.item >= _menuCursor.size)
        {
            _menuCursor.item -= _menuCursor.size;
        }
        setCursorMenuPosition();
    }
    else
    {
        _menuCursor.active = false;
        setCursorPosition(-1, -1);
    }
}

int UMDDisplay::menuCurrentItem()
{
    return _menuCursor.item;
}

void UMDDisplay::initMenuCursor(int layer, int size)
{
    _menuCursor.active = true;
    _menuCursor.size = size;
    _menuCursor.scrollRequired = 0;
    _menuCursor.item = 0;
    if(layer == 0)
    {
        _menuCursor.startLine = 0;
    }
    else
    {
        // find where this menu starts in display line space
        int line = 0;
        for(int prevLayers = 0; prevLayers < layer; prevLayers++)
        {
            line += _layerLength[prevLayers];
        }
        _menuCursor.startLine = line;
    }
    setCursorMenuPosition();
}

void UMDDisplay::fillLayerFromMenu(int layer, int startBufferIndex, int startMenuIndex)
{

    // fill the buffer with menu items, don't override first line
    int menuIndex = startMenuIndex;
    int bufferIndex = startBufferIndex;

    for(int i = 0; i < UMD_DISPLAY_BUFFER_TOTAL_LINES; i++)
    {
        // have we reached the end of the menu items? if so clear the rest of the buffer
        if(menuIndex >= _menu.size())
        {
            //clearLine(bufferIndex);
            return;
        }
        else
        {
            print(layer, _menu[menuIndex++], bufferIndex++, 1);
        }

        // wrap around the buffer, keeping the first line intact
        if(bufferIndex >= UMD_DISPLAY_BUFFER_TOTAL_LINES)
            bufferIndex = 0;
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
    if(this->_cursorPosition.x >= 0 && this->_cursorPosition.y >= 0)
    {
        _display->setCursor(this->_cursorPosition.x, this->_cursorPosition.y);
        _display->print(this->_cursorChar);
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
        _scroll[layer][lineNumber][0] += delta;
        if(_scroll[layer][lineNumber][0] >= UMD_DISPLAY_BUFFER_TOTAL_LINES)
        {
            _scroll[layer][lineNumber][0] = 0;
        }
        else if(_scroll[layer][lineNumber][0] <= 0)
        {
            _scroll[layer][lineNumber][0] = UMD_DISPLAY_BUFFER_TOTAL_LINES-1;
        }
    }
    _needsRedraw = true;
}