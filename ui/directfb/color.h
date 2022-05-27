#ifndef __COLOR_H_
#define __COLOR_H_

#ifndef u8
typedef unsigned char u8;
#endif // u8

class Color
{
public:
    static Color RESET;
    static Color WHITE;
    static Color BLACK;
    static Color GRAY;
    static Color YELLOW;
    
    Color();

    Color(const Color& color);

    Color(u8 red, u8 green, u8 blue, u8 alpha = 255);

    ~Color();

    u8 getRed() const;
    u8 getGreen() const;
    u8 getBlue() const;
    u8 getAlpha() const;

    void setColor(u8 red, u8 green, u8 blue, u8 alpha = 255);

    Color& operator=(const Color& color);
    bool   operator==(const Color& color) const;
    bool   operator!=(const Color& color) const;

private:
    u8 mRed;
    u8 mGreen;
    u8 mBlue;
    u8 mAlpha;
};

inline u8 Color::getRed() const
{
    return mRed;
}

inline u8 Color::getGreen() const
{
    return mGreen;
}

inline u8 Color::getBlue() const
{
    return mBlue;
}

inline u8 Color::getAlpha() const
{
    return mAlpha;
}

inline void Color::setColor(u8 red, u8 green, u8 blue, u8 alpha)
{
    mRed    = red;
    mGreen  = green;
    mBlue   = blue;
    mAlpha  = alpha;
}

#endif /* __COLOR_H_ */
