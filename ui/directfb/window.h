#ifndef __INNO_WINDOW_H_
#define __INNO_WINDOW_H_

#include "directfb.h"

#include "point.h"
#include "size.h"
#include "color.h"
#include "image.h"
#include "font.h"

namespace PrazenUI
{

class Window;

enum
{
    OSD_LAYER_BUTTOM,
    OSD_LAYER_MIDDLE,
    OSD_LAYER_TOP,

    MAX_OSD /* DO NOT MODIFY */
};

class IWindowListener
{
public:
    virtual ~IWindowListener() {}

    virtual void onCreateWindow(Window* window) = 0;
    virtual void onDestroyWindow(Window* window) = 0;
};

class Window
{
public:
    Window(int osdLayer, int x, int y, int width, int height, IWindowListener* listener = NULL);
    
    virtual ~Window();

    IDirectFBWindow*  getDFBWindow();
    IDirectFBSurface* getDFBSurface();

    void setOpacity(int value);

    void drawText(const Font& font, const std::string& text, int x, int y, const Color& color=Color::WHITE, Rectangle* res = NULL);

    void drawImage(Image* image, int x = -1, int y = -1, Rectangle* res = NULL);
    void drawImageStretch(Image* img, Rectangle& dest, Rectangle* res = NULL);
    void drawImageStretchFixed(Image* img, Rectangle& dest, Rectangle* res = NULL);
    
    void drawRectangle(const Rectangle& rect, const Color& color);

    void clear(const Color& color=Color::RESET);
    void clear(const Rectangle& rect, const Color& color=Color::RESET);

    void update();
    void update(const Rectangle& rect);

    int   getScreenWidth() const;
    int   getScreenHeight() const;

    int   getX() const;
    int   getY() const;
    int   getWidth() const;
    int   getHeight() const;

    Size  getLayerSize() const;    
    Size  getSize() const;    
    Point getPos() const;

    void  moveTo(int x, int y);

    bool isVisible();
    void show();
    void hide();

protected:
    IDirectFBWindow*  mDFBWindow;
    IDirectFBSurface* mDFBSurface;
    
    int mOsdLayer;

    int mX;
    int mY;
    int mWidth;
    int mHeight;

    int mOpacity;
    
    IWindowListener*  mListener;

    bool init();
    void deinit();
};

} // namespace PrazenUI

#endif /* __INNO_WINDOW_H_ */
