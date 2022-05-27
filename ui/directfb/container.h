#ifndef __CONTAINER_H_
#define __CONTAINER_H_

#include <list>

#include "event_loop.h"
#include "window.h"

#include "color.h"
#include "rectangle.h"
#include "image.h"
#include "font.h"

#include "widget.h"

class Container;
class IWidget;

class IContainerListener
{
public:
    virtual ~IContainerListener() { };

    virtual void onCreate(Container* container) = 0;
    virtual void onShow(Container* container) = 0;
    virtual void onHide(Container* container) = 0;
    virtual void onDestroy(Container* container) = 0;
};

typedef enum
{
    CONTAINER_REUEST_SHOW,
    CONTAINER_REUEST_HIDE,
    CONTAINER_REUEST_FOCUS,

    CONTAINER_REUEST_MESSAGE,

    MAX_CONTAINER_EVENT
} ContainerEvent_e;

class ContainerEvent : public IFdWatcher
{
public:
    ContainerEvent();
    ~ContainerEvent();

    int getFD();

    int post(int eventID, Container* container);
    int postMessage(Container* container, int messageId, void* param);

private:
    int mPipe[2];

    bool onFdReadable(int fd);
};

class Container : public IWindowListener
{
    friend class ContainerEvent;

public:
    Container(int osdLayer);
    Container(int osdLayer, int width, int height);
    Container(int osdLayer, int x, int y, int width, int height);
    virtual ~Container();

    void addWidget(IWidget* widget);
    void removeWidget(IWidget* widget);

    void setListener(IContainerListener* listener);
    
    int getScreenWidth() const;
    int getScreenHeight() const;

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;

    void setBackground(const Color& color, bool isUpdate = false);
    void setBackground(const Image& img, bool isUpdate = false);
    void redrawBackground(const Rectangle& rect, bool isUpdate = false);

    void drawRectangle(const Rectangle& rect, const Color& color);
    void drawImage(Image* image, int x = -1, int y = -1, Rectangle* res = NULL);
    void drawImageStretch(Image* image, Rectangle& dest, Rectangle* res = NULL);
    void drawImageStretchFixed(Image* image, Rectangle& dest, Rectangle* res = NULL);

    void drawText(const Font& font, const std::string& text, int x, int y, const Color& color=Color::WHITE, Rectangle* res = NULL);

    void clear();
    void clear(const Rectangle& rect);

    void redrawWidgets();

    void update();
    void update(const Rectangle& rect);

    void requestFocus();

    void sendMessage(int id, void* param);

    virtual void setFocus(bool focus);
    virtual bool isFocused() const;
    virtual void onFocusChanged(bool focus);

    virtual bool isVisible();
    virtual void show();
    virtual void hide();
    virtual void onMessageReceived(int id, void* param);

protected:
    IContainerListener* mContainerListener;

    int         mOsdLayer;
    int         mX;
    int         mY;
    int         mWidth;
    int         mHeight;
    bool        mVisible;
    bool        mFocused;

    Window*    mWindow;
    Image*     mBgImg;
    Color*     mBgColor;

    typedef std::list<IWidget*> WidgetList;
    WidgetList mWidgetList;

private:
    void onCreateWindow(Window* window);
    void onDestroyWindow(Window* window);

    void _show();
    void _hide();
};

inline void Container::setListener(IContainerListener* listener) 
{
    mContainerListener = listener;
}

inline int Container::getX() const
{
    return mX;
}

inline int Container::getY() const
{
    return mY;
}

inline int Container::getWidth() const
{
    return mWidth;
}

inline int Container::getHeight() const
{
    return mHeight;
}

inline void Container::drawText(const Font& font, const std::string& text, int x, int y, const Color& color, Rectangle* res)
{
    if(mWindow == NULL)
        return;
    
    mWindow->drawText(font, text, x, y, color, res);
}

inline void Container::drawImage(Image* image, int x, int y, Rectangle* res)
{
    if(mWindow == NULL)
        return;
    
    mWindow->drawImage(image, x, y, res);
}

inline void Container::drawImageStretch(Image* image, Rectangle& dest, Rectangle* res)
{
    if(mWindow == NULL)
        return;
    
    mWindow->drawImageStretch(image, dest, res);
}

inline void Container::drawImageStretchFixed(Image* image, Rectangle& dest, Rectangle* res)
{
    if(mWindow == NULL)
        return;
    
    mWindow->drawImageStretchFixed(image, dest, res);
}

inline void Container::drawRectangle(const Rectangle& rect, const Color& color)
{
    if(mWindow == NULL)
        return;
 
    mWindow->drawRectangle(rect, color);   
}

inline void Container::clear()
{
    if(mWindow == NULL)
        return;

    mWindow->clear();
}

inline void Container::clear(const Rectangle& rect)
{
    if(mWindow == NULL)
        return;

    mWindow->clear(rect);
}

inline void Container::update()
{
    if(mWindow == NULL)
        return;
    
    mWindow->update();
}

inline void Container::update(const Rectangle& rect)
{
    if(mWindow == NULL)
        return;
    
    mWindow->update(rect);
}

inline bool Container::isFocused() const
{
    return mFocused;
}

inline bool Container::isVisible()
{
    if(mWindow == NULL)
        return mVisible;

    return mWindow->isVisible();
}


#endif // __CONTAINER_H_
