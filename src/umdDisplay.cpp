
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

    if(!_display->begin(SSD1306_SWITCHCAPVCC, 0x3c)) { 
        return false;
    }

    for(int i = 0; i < OLED_MAX_LINES_PER_SCREEN; i++){
        scroll[i][0] = i;
        scroll[i][1] = 0;
    }

    _display->display();
    delay(2000); // Pause for 2 seconds
    _display->clearDisplay();
    _display->setTextSize(1);             // Normal 1:1 pixel scale
    _display->setTextColor(WHITE);        // Draw white text
    _display->display();
    _needsRedraw = false;
    return true;
}

void UMDDisplay::setCursorChar(char c){
    _cursorChar = c;
    this->_needsRedraw = true;
}

void UMDDisplay::setCursorPosition(int x, int y){
    if(x < OLED_MAX_CHARS_PER_LINE){
        this->_cursorPosition.x = x * OLED_FONT_WIDTH;
    }else{
        this->_cursorPosition.x = -1;
    }
    
    if(y < OLED_MAX_LINES_PER_SCREEN){
        this->_cursorPosition.y = y * OLED_FONT_HEIGHT;
    }else{
        this->_cursorPosition.y = -1;
    }
    this->_needsRedraw = true;
}

void UMDDisplay::clear(void)
{
    for(int line = 0; line < UMD_DISPLAY_BUFFER_TOTAL_LINES; line++)
    {
        this->clearLine(line);
    }
    _needsRedraw = true;
}

void UMDDisplay::clearLine(int lineNumber)
{
    for(int chr = 0; chr < UMD_DISPLAY_BUFFER_CHARS_PER_LINE; chr++)
    {
        this->buffer[lineNumber][chr] = ' ';
    }
    _needsRedraw = true;
}

void UMDDisplay::printf(int lineNumber, const char *format, ...){
    va_list args;
    va_start(args, format);
    vsnprintf(buffer[lineNumber], UMD_DISPLAY_BUFFER_CHARS_PER_LINE, format, args);
    va_end(args);
    _needsRedraw = true;
}

void UMDDisplay::printf(int lineNumber, const __FlashStringHelper *format, ...){
    va_list args;
    va_start(args, format);
    vsnprintf(buffer[lineNumber], UMD_DISPLAY_BUFFER_CHARS_PER_LINE, (const char *)format, args);
    va_end(args);
    _needsRedraw = true;
}

void UMDDisplay::print(const char characters[], int lineNumber, int printPos)
{
    int i = 0;
    bufferNextPos[lineNumber] = printPos;

    if(bufferNextPos[lineNumber] >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE){
        bufferNextPos[lineNumber] = 0;
    }

    while(characters[i] != '\0'){
        // wrap around if required
        if(i >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE )
        {
            bufferNextPos[lineNumber] = 0;
        }
        this->buffer[lineNumber][bufferNextPos[lineNumber]++] = characters[i++];
    }
}

void UMDDisplay::print(const char characters[], int lineNumber)
{
    this->print(characters, lineNumber, bufferNextPos[lineNumber]);
}

void UMDDisplay::print(int number, int lineNumber){
    std::string tmp = std::to_string(number);
    const char *num_char = tmp.c_str();
    this->print(num_char, lineNumber);
}

void UMDDisplay::initMenu(const char *menuItems[], int size){
    _menu.clear();
    for(int i = 0; i < size; i++){
        _menu.push_back(menuItems[i]);
    }
    
    _menuItemPtr = 0;

    // fill the buffer with menu items, don't override first line
    for(int i = 1; i < UMD_DISPLAY_BUFFER_TOTAL_LINES; i++){
        this->print(_menu[i-1], i);
    }
    _needsRedraw = true;
}

void UMDDisplay::redraw(void)
{
    if(!_needsRedraw){
        return;
    }
    
    char lineChars[OLED_MAX_CHARS_PER_LINE+1]; // +1 for terminator
    int bufferPos, lineFromBuffer;

    _display->clearDisplay();

    for(int lineOnDisplay = 0; lineOnDisplay < OLED_MAX_LINES_PER_SCREEN; lineOnDisplay++)
    {
        lineFromBuffer = scroll[lineOnDisplay][0];

        _display->setCursor(0, OLED_LINE_NUMBER(lineOnDisplay));
        bufferPos = scroll[lineOnDisplay][1];
        // file the lineChars buffer
        for(int pos = 0; pos < OLED_MAX_CHARS_PER_LINE; pos++)
        {
            // wrap around if required
            if(bufferPos >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE )
            {
                bufferPos = 0;
            }
            lineChars[pos] = buffer[lineFromBuffer][bufferPos++];
        }
        lineChars[OLED_MAX_CHARS_PER_LINE] = '\0'; // Ensure null-termination
        _display->print(lineChars);
    }

    if(this->_cursorPosition.x >= 0 && this->_cursorPosition.y >= 0){
        _display->setCursor(this->_cursorPosition.x, this->_cursorPosition.y);
        _display->print(this->_cursorChar);
    }

    _display->display();
    for(int i = 0; i < UMD_DISPLAY_BUFFER_TOTAL_LINES; i++){
        bufferNextPos[i] = 0;
    }
    _needsRedraw = false;
}

void UMDDisplay::scrollX(int lineNumber, int delta){
    if(delta > UMD_DISPLAY_BUFFER_CHARS_PER_LINE){
        return;
    }

    scroll[lineNumber][1] += delta;
    if(scroll[lineNumber][1] >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE)
    {
        scroll[lineNumber][1] -= UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
    }
    else if(scroll[lineNumber][1] < 0)
    {
        scroll[lineNumber][1] += UMD_DISPLAY_BUFFER_CHARS_PER_LINE;
    }
    _needsRedraw = true;
}

void UMDDisplay::scrollY(int delta){
    this->scrollY(delta, 0);
    _needsRedraw = true;
}

void UMDDisplay::scrollY(int delta, int startIndex){
    if(delta > UMD_DISPLAY_BUFFER_TOTAL_LINES){
        return;
    }

    for(int lineNumber = startIndex; lineNumber < OLED_MAX_LINES_PER_SCREEN; lineNumber++){
        scroll[lineNumber][0] += delta;
        if(scroll[lineNumber][0] >= UMD_DISPLAY_BUFFER_TOTAL_LINES){
            scroll[lineNumber][0] = startIndex;
        }
        else if(scroll[lineNumber][0] <= startIndex)
        {
            scroll[lineNumber][0] = UMD_DISPLAY_BUFFER_TOTAL_LINES-1;
        }
    }
    _needsRedraw = true;
}