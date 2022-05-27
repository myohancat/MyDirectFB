#ifndef __FONT_H_
#define __FONT_H_

#include <string>
#include "directfb.h"
#include "size.h"


class Font
{
public:

    enum FontStyle_e
    {
        FONT_STYLE_PLAIN  = 0x00,
        FONT_STYLE_ITALIC = 0x01,
        FONT_STYLE_BOLD   = 0x02
    };

    Font(const std::string& path, int size);
    Font(const std::string& path, int size, FontStyle_e style);

    ~Font();

    Size getExtents(const std::string& text) const;    

    Size getGlyphExtents(unsigned int c) const;

    void stringBreak(const char* text, int offset, int maxWidth, int* lineWidth, int* length, const char** nextLine);

    const std::string getEllipsisString(const std::string& str, int maxWidth);

    IDirectFBFont* getDFBFont() const;

private:
    std::string    mPath;
    int            mSize;
    FontStyle_e    mStyle;

    IDirectFBFont* mDFBFont;

    bool loadFont();
};

#endif /* __FONT_H_ */
