#include "rectangle.h"

namespace PrazenUI
{

Rectangle::Rectangle()
          : mX(0),
            mY(0),
            mWidth(0),
            mHeight(0)
{
}


Rectangle::Rectangle(int x, int y, int width, int height)
          : mX(x),
            mY(y),
            mWidth(width),
            mHeight(height)
{
}

Rectangle::Rectangle(int x, int y, const Size& size)
          : mX(x),
            mY(y),
            mWidth(size.getWidth()),
            mHeight(size.getHeight())
{
}

Rectangle::Rectangle(const Point& point, const Size& size)
          : mX(point.getX()),
            mY(point.getY()),
            mWidth(size.getWidth()),
            mHeight(size.getHeight())
{
}


Rectangle::Rectangle(const Rectangle& rect)
          : mX(rect.getX()),
            mY(rect.getY()),
            mWidth(rect.getWidth()),
            mHeight(rect.getHeight())
{
}


Rectangle::~Rectangle()
{
}


bool Rectangle::contains(const Point& point) const
{
    return contains(point.getX(), point.getY());
}


bool Rectangle::contains(int x, int y) const
{
    return ((x >= mX && x <= (mX + mWidth)) &&
            (y >= mY && y <= (mY + mHeight)));
}


bool Rectangle::contains(const Rectangle& rect) const
{
    return ((rect.getX() >= mX) && (rect.getX() + rect.getWidth() <= mX + mWidth) &&
            (rect.getY() >= mY) && (rect.getY() + rect.getHeight() <= mY + mHeight)); 
}


#if 0 /* TODO IMPLEMENTS HERE */
bool Rectangle::intersects(const Rectangle& rect) const
{
    // TODO Implements Here
    return true;
}


Rectangle Rectangle::intersection(const Rectangle& rect) const
{
    // TODO Implements Here
    return Rectangle(0, 0, 0, 0);
}
#endif

#define MIN(x, y) ((x>y)?y:x)
#define MAX(x, y) ((x>y)?x:y)

void Rectangle::united(const Rectangle& rect) 
{
    if(!rect.isValid())
        return;

    if(!isValid())
    {
        setRectangle(rect);
        return;
    }

    int x1 = MIN(mX, rect.getX());
    int y1 = MIN(mY, rect.getY());
    
    int x2 = MAX(mX + mWidth, rect.getX() + rect.getWidth());
    int y2 = MAX(mY + mHeight, rect.getY() + rect.getHeight());

    mX = x1;
    mY = y1;
    mWidth = x2 - x1;
    mHeight = y2 - y1;
}

Rectangle& Rectangle::operator=(const Rectangle &rect)
{
    if(this != &rect)
    {
        mX = rect.mX;
        mY = rect.mY;
        mWidth = rect.mWidth;
        mHeight = rect.mHeight;
    }

    return *this;
}

bool Rectangle::operator==(const Rectangle &rect)
{
    return (mX == rect.mX) && (mY == rect.mY) && (mWidth == rect.mWidth) && (mHeight == rect.mHeight);
}

bool Rectangle::operator!=(const Rectangle &rect)
{
    return !(*this == rect);
}


} // namespace PrazenUI
