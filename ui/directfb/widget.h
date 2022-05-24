#ifndef __WIDGET_H_
#define __WIDGET_H_

#include "rectangle.h"
#include "container.h"

namespace PrazenUI
{

class Container;

class IWidget
{
public:
    virtual ~IWidget() { }

    virtual void setContainer(Container* container) = 0;

    virtual int  getX() = 0;
    virtual int  getY() = 0;
    virtual int  getWidth() = 0;
    virtual int  getHeight() = 0;
    virtual Rectangle getRectangle() = 0;

    virtual bool contains(int posX, int posY) = 0;

    virtual void setFocus(bool focus) = 0;
    virtual bool isFocused() = 0;

    virtual void setEnable(bool enable) = 0;
    virtual bool isEnabled() = 0;

    virtual void paint() = 0;
    virtual void paint(const Rectangle& rect) = 0;
};


} // namespace PrazenUI

#endif /* __WIDGET_H_ */
