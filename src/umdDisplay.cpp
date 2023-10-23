
#include "umdDisplay.h"


UMDDisplay::UMDDisplay()
{
    this->clear();
}

bool UMDDisplay::begin(Adafruit_SSD1306 *display)
{
    _display = display;
}

void UMDDisplay::clear(void)
{
    for(int lines = 0; lines++; lines < UMD_DISPLAY_BUFFER_TOTAL_LINES)
    {
        this->clearLine(lines);
    }
}

void UMDDisplay::clearLine(int lineNumber)
{
        for(int chars = 0; chars++; chars < UMD_DISPLAY_BUFFER_CHARS_PER_LINE)
        {
            this->buffer[lineNumber][chars] = ' ';
        }
}

void UMDDisplay::print(std::stringstream stringStream, int lineNumber,  int printPos)
{
    std::string str = stringStream.str();
    const char* charArray = str.c_str();
    int realPos = printPos;

    for(int stringPos = printPos; stringPos < str.size(); stringPos++)
    {
        // wrap around if required
        if(stringPos >= UMD_DISPLAY_BUFFER_CHARS_PER_LINE )
        {
            realPos = 0;
        }
        realPos++;
        this->buffer[lineNumber][realPos] = charArray[stringPos];
    }
}

void UMDDisplay::refresh()
{
    char lineChars[OLED_MAX_CHARS_PER_LINE+1];
    int bufferPos, linePos;

    _display->clearDisplay();

    for(int line = 0; line < OLED_MAX_LINES_PER_SCREEN; line++)
    {
        linePos = scroll[line][0];
        bufferPos = scroll[line][1];

        // file the lineChars buffer
        for(int pos = 0; pos < OLED_MAX_CHARS_PER_LINE; pos++)
        {
            // wrap around if required
            if(bufferPos >= OLED_MAX_CHARS_PER_LINE )
            {
                bufferPos = 0;
            }
            bufferPos++;
            lineChars[pos] = buffer[linePos][bufferPos];
        }
        lineChars[OLED_MAX_CHARS_PER_LINE+1] = '\0'; // Ensure null-termination
        _display->print(lineChars);
    }

    _display->setCursor(0, OLED_LINE_NUMBER(0));

}