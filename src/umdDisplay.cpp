
#include "umdDisplay.h"


UMDDisplay::UMDDisplay()
{
    this->clear();
}

void UMDDisplay::begin(Adafruit_SSD1306 *display)
{
    _display = display;

    for(int i = 0; i < OLED_MAX_LINES_PER_SCREEN; i++){
        scroll[i][0] = i;
        scroll[i][1] = 0;
    }
}

void UMDDisplay::clear(void)
{
    for(int line = 0; line < UMD_DISPLAY_BUFFER_TOTAL_LINES; line++)
    {
        this->clearLine(line);
    }
}

void UMDDisplay::clearLine(int lineNumber)
{
    for(int chr = 0; chr < UMD_DISPLAY_BUFFER_CHARS_PER_LINE; chr++)
    {
        this->buffer[lineNumber][chr] = ' ';
    }
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

void UMDDisplay::redraw()
{
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

    _display->display();
    for(int i = 0; i < UMD_DISPLAY_BUFFER_TOTAL_LINES; i++){
        bufferNextPos[i] = 0;
    }
}

void UMDDisplay::scrollLineX(int lineNumber, int delta){
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
}

void UMDDisplay::scrollRotateDown(int delta){
    this->scrollRotateDown(delta, 0);
}

void UMDDisplay::scrollRotateDown(int delta, int startIndex){
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
}