#include "font.h"

#include "dfb_manager.h"
#include "log.h"

namespace PrazenUI
{

Font::Font(const std::string& path, int size)
         : mPath(path),
           mSize(size),
           mStyle(FONT_STYLE_PLAIN),
           mDFBFont(NULL)
{
    loadFont();

}

Font::Font(const std::string& path, int size, FontStyle_e style)
         : mPath(path),
           mSize(size),
           mStyle(style),
           mDFBFont(NULL)
{
    loadFont();
}

Font::~Font()
{
    if(mDFBFont != NULL)
    {
        mDFBFont->Release(mDFBFont);
        mDFBFont = NULL;
    }
}


Size Font::getExtents(const std::string& text) const
{
    DFBRectangle rect;
    
    mDFBFont->GetStringExtents(mDFBFont, text.c_str(), -1, &rect, NULL);

    return Size(rect.w, rect.h);
}


Size Font::getGlyphExtents(unsigned int c) const
{
    DFBRectangle rect;
    
    mDFBFont->GetGlyphExtents(mDFBFont, c, &rect, NULL);

    return Size(rect.w, rect.h);
}

void Font::stringBreak(const char* text, int offset, int maxWidth, int* lineWidth, int* length, const char** nextLine)
{
    if(mDFBFont->GetStringBreak(mDFBFont, text, offset, maxWidth, lineWidth, length, nextLine) != DFB_OK)
    {
        *lineWidth = 0;
        *length = 0;
        *nextLine = 0;
    }
}

const std::string Font::getEllipsisString(const std::string& str, int maxWidth)
{
    int         lineWidth = 0;
    int         length = 0;
    const char* nextLine = 0;

    Size size = getExtents(str);
    if(size.getWidth() <= maxWidth)
        return str;

    Size ellipsisSize = getExtents("...");
    if(ellipsisSize.getWidth() >= maxWidth)
    {
        // TODO
        return std::string("...");
    }

    stringBreak(str.c_str(), -1, maxWidth - ellipsisSize.getWidth(), &lineWidth, &length, &nextLine);
    
    return str.substr(0, length) + "...";
}


IDirectFBFont* Font::getDFBFont() const
{
    return mDFBFont;
}

bool Font::loadFont()
{
    DFBFontDescription desc;

    if(mDFBFont)
        return true;

    desc.flags = (DFBFontDescriptionFlags)(DFDESC_HEIGHT | DFDESC_ATTRIBUTES);
    desc.height = mSize;
    desc.attributes = DFFA_NONE;
#if 1 // support outline
    desc.flags = (DFBFontDescriptionFlags)(desc.flags | DFDESC_OUTLINE_WIDTH | DFDESC_OUTLINE_OPACITY); 
    desc.outline_width = 100;
    desc.outline_opacity = 255;

    desc.attributes = DFFA_OUTLINED;
#endif

    if(mStyle & FONT_STYLE_ITALIC) desc.attributes = DFFA_STYLE_ITALIC; // <! TBD. Check it !!!
    if(mStyle & FONT_STYLE_BOLD) desc.attributes = DFFA_STYLE_BOLD; // <! TBD. Check it !!!

    DFBResult ret = DFBManager::getInstance().getDFB()->CreateFont(DFBManager::getInstance().getDFB(), mPath.c_str(), &desc, &mDFBFont);
    if(ret != DFB_OK)
    {
        LOG_ERROR("cannot create font !\n");
        return false;
    }

    return true;
}


} // namespace PrazenUI
