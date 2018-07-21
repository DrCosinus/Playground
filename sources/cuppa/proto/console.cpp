#include "console.hpp"
#include "os.hpp"

#include "../cuppa.hpp"

#include <iostream>
#include <string>

using namespace cuppa;

namespace proto
{
    struct OS;

    void Console::setup(OS& _os)
    {
        os = &_os;
        font = loadFont("Consolas", 14);
        auto p = textSize("1");
        std::cout << p.width.getAs<float>() << "x" << p.height.getAs<float>() << std::endl;
        p = textSize("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n0");
        std::cout << p.width.getAs<float>() << "x" << p.height.getAs<float>() << std::endl;
    }

    Color toColor(unsigned char colorCode)
    {
        switch(colorCode)
        {
            case 1:  return Lime;
            case 2:  return Red;
            case 3:  return Yellow;
            case 4:  return White;
            default: return Blue;
        }
    }

    void Console::draw()
    {
        if (idleTime > 0)
        {
            if (idleTime > 16)
                idleTime -= 16;
            else
                idleTime = 0;
        }
        updateScripts();
        textFont(font);
        fill(Lime);
        int index = 0;
        for(auto& c: buffer)
        {
            int row = index / cols;
            int col = index - row * cols;
            fill(toColor(c.color));
            text({ &c.character, 1 }, { 8_px * col, 16_px * row});
            ++index;
        }
        fill(toColor(caret_color));
        caretBlinkCounter = (caretBlinkCounter+1)%60;
        if (caretBlinkCounter < 30)
        {
            rect({ 6_px+ 8_px * caret_col, 8_px+16_px * caret_row}, { 8_px, 16_px });
        }
    }

    bool Console::ready()
    {
        return scripts.empty() && idleTime==0;
    }

    void Console::writeLine(std::size_t row, std::string_view line)
    {
        auto subline = line.substr(0, cols);
        std::transform(std::begin(subline), std::end(subline), buffer+row*cols, [caret_color=caret_color](auto c){ return cell{ c, caret_color }; } );
    }

    void Console::writeScript(std::string_view line)
    {
        scripts.push(line);
    }

    void Console::scrollup()
    {
        std::copy( buffer + cols, buffer + rows * cols, buffer);
        std::fill( buffer + (rows-1)*cols, buffer + rows * cols, cell{ ' ', caret_color });
        if (caret_row>0)
            --caret_row;
    }

    void Console::scrolldown()
    {
        std::copy( buffer + (caret_row+1) * cols, buffer + (rows-1) * cols, buffer + (caret_row+2) * cols);
        std::fill( buffer + (caret_row+1)*cols, buffer + (caret_row+2) * cols, cell{ ' ', caret_color });
    }

    void Console::clearScreen()
    {
        std::fill(buffer, buffer+cols*rows, cell{ ' ', caret_color});
        caret_col = caret_row = 0;
    }

    void Console::triggerEvent(std::size_t eventID)
    {
        os->onConsoleEvent(eventID);
    }

    void Console::updateScripts()
    {
        if (idleTime == 0 && !scripts.empty())
        {
            auto& line = scripts.front();

            for(int i = 0; !line.empty() && idleTime==0; ++i)
            {
                // commands are /Lxyz: where
                // - L is a single character coding the command
                // - xyz is an optional positive integral value
                // - colon character
                // consequently command are at least 3 characters long
                if (line[0]=='/' && line.size()>2)
                {
                    auto j = 2u;
                    std::size_t value = 0;
                    while (line.size()>j && line[j]>='0' && line[j]<='9')
                    {
                        value = 10 * value + (line[j++]-'0');
                    }
                    if (line.size()>j && line[j]==':')
                    {
                        switch(line[1])
                        {
                            case 'P': // /Pxyz: pause immediately for xyz milliseconds
                                idleTime = value;
                                break;
                            case 'D': // /Dxyz: setup delay time between each characters
                                characterTime = value;
                                break;
                            case 'C': // /Cxyz: set the following characters color
                                caret_color = static_cast<decltype(caret_color)>(value);
                                break;
                            case 'N': // /N: new line
                                caret_col = 0;
                                caret_row++;
                                break;
                            case 'S': // /S: scroll up
                                scrollup();
                                idleTime = characterTime;
                                break;
                            case 'E': // /Exyz: trigger event xyz
                                triggerEvent(value);
                                break;
                        }
                        line = line.substr(j+1);
                        continue;
                    }
                }
                buffer[caret_col+caret_row*cols] = { line[0], caret_color };
                caret_col++;
                if (caret_col==cols)
                {
                    caret_col = 0;
                    caret_row++;
                }
                line = line.substr(1);
                idleTime = characterTime;
                caretBlinkCounter = 0;
            }
            if (line.empty())
                scripts.pop();
        }
    }

    void Console::pushChar(char c)
    {
        writeLine( 1, std::to_string(c));
        if (ready())
        {
            caretBlinkCounter = 0;
            if (c >= ' ' || c < 0)
            {
                buffer[caret_col+caret_row*cols] = { c, caret_color };
                caret_col++;
                if(caret_col>=cols)
                {
                    caret_col = 0;
                    caret_row++;
                    if (caret_row>=rows)
                        scrollup();
                }
            }
        }
    }

    void Console::pushKeyCode(short keyCode)
    {
        writeLine( 0, std::to_string(keyCode));
        if (ready())
        {
            switch(keyCode)
            {
            case 8: // BACK
                if (caret_row != 0 || caret_col != 0)
                {
                    if (caret_col==0)
                    {
                        caret_col = cols;
                        caret_row--;
                    }
                    caret_col--;
                    buffer[caret_col+cols*caret_row].character = ' ';
                }
                break;
            case 46: // DEL
                if (caret_col<cols-1)
                    std::copy(buffer+caret_row*cols+caret_col+1,buffer+(caret_row+1)*cols,buffer+caret_row*cols+caret_col);
                buffer[(caret_row+1)*cols-1].character = ' ';
                break;
            case 13: // RETURN
                {
                    std::string str(cols, ' ');
                    std::transform(buffer+caret_row*cols, buffer+(caret_row+1)*cols,std::begin(str), [](cell c) { return c.character; });
                    auto strview = std::string_view(str);
                    strview.remove_prefix(strview.find_first_not_of(' '));
                    strview.remove_suffix(strview.size()-1-strview.find_last_not_of(' '));
                    scrolldown();
                    caret_col = 0;
                    caret_row++;
                    if (caret_row == rows)
                        scrollup();
                    os->interpret(strview);
                }
                break;
            case 37:    if (caret_col == 0) {   caret_col = cols;    }   caret_col--;   break;
            case 38:    if (caret_row == 0) {   caret_row = rows;    }   caret_row--;   break;
            case 39:    caret_col++; if (caret_col == cols) {   caret_col = 0;    }     break;
            case 40:    caret_row++; if (caret_row == rows) {   caret_row = 0;    }     break;
            }
        }
    }

} // namespace proto