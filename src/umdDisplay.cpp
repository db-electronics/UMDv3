
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
    for(int lines = 0; lines < UMD_DISPLAY_BUFFER_TOTAL_LINES; lines++)
    {
        this->clearLine(lines);
    }
}

void UMDDisplay::clearLine(int lineNumber)
{
        for(int chars = 0; chars < UMD_DISPLAY_BUFFER_CHARS_PER_LINE; chars++)
        {
            this->buffer[lineNumber][chars] = ' ';
        }
}

void UMDDisplay::print(const char characters[], int lineNumber,  int printPos)
{
    int realPos = printPos;
    int i = 0;

    while(characters[i] != '\0'){
        // wrap around if required
        if(i >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE )
        {
            realPos = 0;
        }
        this->buffer[lineNumber][realPos++] = characters[i++];
    }
}

void UMDDisplay::redraw()
{
    char lineChars[OLED_MAX_CHARS_PER_LINE+1];
    int bufferPos, linePos;

    _display->clearDisplay();

    for(int line = 0; line < OLED_MAX_LINES_PER_SCREEN; line++)
    {
        if(line == 2){
            SerialUSB.println("line2");
        }

        linePos = scroll[line][0];
        bufferPos = scroll[line][1];

        _display->setCursor(0, OLED_LINE_NUMBER(line));

        // file the lineChars buffer
        for(int pos = 0; pos < OLED_MAX_CHARS_PER_LINE; pos++)
        {
            // wrap around if required
            if(bufferPos >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE )
            {
                bufferPos = 0;
            }
            lineChars[pos] = buffer[linePos][bufferPos++];
        }
        lineChars[OLED_MAX_CHARS_PER_LINE+1] = '\0'; // Ensure null-termination
        _display->print(lineChars);
    }
    _display->display();
}

void UMDDisplay::scrollLine(int lineNumber, int delta){
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