#include "point.h"

namespace PrazenUI
{

Point::Point()
      :mX(-1),
       mY(-1)
{
}


Point::Point(const Point& p)
      :mX(p.mX),
       mY(p.mY)
{
}


Point::Point(int x, int y)
      :mX(x),
       mY(y)
{
}


void Point::setX(int x)
{
    mX = x;
}


void Point::setY(int y)
{
    mY = y;
}


Point& Point::operator=(const Point& p)
{
    if(this != &p)
    {
        mX = p.mX;
        mY = p.mY;
    }

    return *this;
}


bool Point::operator==(const Point& p)
{
    return (mX == p.mX) && (mY == p.mY);
}


bool   Point::operator!=(const Point& p)
{
    return (mX != p.mX) && (mY != p.mY);
}


} // PrazenUI
