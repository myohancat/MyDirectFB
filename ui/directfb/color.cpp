#include "color.h"

namespace PrazenUI
{

// Static Definition
Color Color::RESET(0x00, 0x00, 0x00, 0x00);
Color Color::WHITE(0xFF, 0xFF, 0xFF, 0xFF);
Color Color::BLACK(0x00, 0x00, 0x00, 0xFF);
Color Color::GRAY(0x80, 0x80, 0x80, 0xFF);
Color Color::YELLOW(0xFF, 0xFF, 0x00, 0xFF);

Color::Color()
          : mRed(255),
            mGreen(255),
            mBlue(255),
            mAlpha(255)
{
}

Color::Color(const Color& color)
          : mRed(color.getRed()),
            mGreen(color.getGreen()),
            mBlue(color.getBlue()),
            mAlpha(color.getAlpha())
{
}

Color::Color(u8 red, u8 green, u8 blue, u8 alpha)
          : mRed(red),
            mGreen(green),
            mBlue(blue),
            mAlpha(alpha)
{
}

Color::~Color()
{
}

Color& Color::operator=(const Color& color)
{
    if(this != &color)
    {
        mRed = color.getRed();
        mGreen = color.getGreen();
        mBlue = color.getBlue();
        mAlpha = color.getAlpha();
    }

    return *this;
}

bool Color::operator==(const Color& color) const
{
    return ((mRed == color.getRed()) && (mGreen == color.getGreen()) &&
            (mBlue == color.getBlue()) && (mAlpha == color.getAlpha()));
}

bool Color::operator!=(const Color& color) const
{
    return ((mRed != color.getRed()) || (mGreen != color.getGreen()) || 
            (mBlue != color.getBlue()) || (mAlpha != color.getAlpha()));
}

} // namespace PrazenUI
