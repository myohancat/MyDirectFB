#include "window.h"

#include "dfb_manager.h"
#include "log.h"

namespace PrazenUI
{

Window::Window(int osdLayer, int x, int y, int width, int height, IWindowListener* listener)
           : mDFBWindow(NULL),
             mDFBSurface(NULL),
             mOsdLayer(osdLayer),
             mX(x),
             mY(y),
             mWidth(width),
             mHeight(height),
             mOpacity(255),
             mListener(listener)
{
    init();
}


Window::~Window()
{
    deinit();
}

void Window::setOpacity(int value)
{
    if(value < 0)
        value = 0;
    if(value > 255)
        value = 255;

    mOpacity = value;
}

IDirectFBWindow* Window::getDFBWindow() 
{
    return mDFBWindow;
}

IDirectFBSurface* Window::getDFBSurface()
{
    return mDFBSurface;
}

int Window::getScreenWidth() const
{
    return DFBManager::getInstance().getLayerSize().getWidth();
}

int Window::getScreenHeight() const
{
    return DFBManager::getInstance().getLayerSize().getHeight();
}

int Window::getWidth() const
{
    int w = 0, h = 0;
    
    if(mDFBWindow != NULL)
        mDFBWindow->GetSize(mDFBWindow, &w, &h);

    return w;
}

int Window::getHeight() const
{
    int w = 0, h = 0;
    
    if(mDFBWindow != NULL)
        mDFBWindow->GetSize(mDFBWindow, &w, &h);

    return h;
}

Size Window::getLayerSize() const
{
    return DFBManager::getInstance().getLayerSize();
}

Size Window::getSize() const
{
    int w = 0, h = 0;
    
    if(mDFBWindow != NULL)
        mDFBWindow->GetSize(mDFBWindow, &w, &h);

    return Size(w, h);
}

Point Window::getPos() const
{
    int x = 0, y = 0;

    if(mDFBWindow != NULL)
        mDFBWindow->GetPosition(mDFBWindow, &x, &y);

    return Point(x, y);
}

void Window::drawText(const Font& font, const std::string& text, int x, int y, const Color& color, Rectangle* res)
{
    if(mDFBSurface == NULL)
        return;

    Size size = font.getExtents(text);

    mDFBSurface->SetFont(mDFBSurface, font.getDFBFont());
#if 1    
    mDFBSurface->SetColor(mDFBSurface, color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
#else
    DFBColorID  color_ids[2] = { DCID_PRIMARY, DCID_OUTLINE };
    DFBColor    colors[2];
    colors[0].a = color.getAlpha();
    colors[0].r = color.getRed();
    colors[0].g = color.getGreen();
    colors[0].b = color.getBlue();
    
    colors[1].a = 0xFF;
    colors[1].r = 0x99;
    colors[1].g = 0x99;
    colors[1].b = 0x99;

    mDFBSurface->SetColors(mDFBSurface, color_ids, colors, 2);
#endif

    if(x < 0)
        x = (mWidth - size.getWidth()) / 2;

    if(y < 0)
        y = (mHeight - size.getHeight()) / 2; 

    mDFBSurface->DrawString(mDFBSurface, text.c_str(), -1, x, y, DSTF_TOPLEFT);

    if(res)
        res->setRectangle(x, y, size);    
}

void Window::drawImage(Image* img, int x, int y, Rectangle* res)
{
    if(mDFBSurface == NULL)
        return;

    if(img == NULL)
        return;

    if(x < 0)
        x = (mWidth - img->getWidth()) / 2;

    if(y < 0)
        y = (mHeight - img->getHeight()) / 2;

    mDFBSurface->SetBlittingFlags(mDFBSurface, DSBLIT_BLEND_ALPHACHANNEL);
    //mDFBSurface->SetPorterDuff(mDFBSurface, DSPD_SRC_OVER);
    mDFBSurface->SetPorterDuff(mDFBSurface, DSPD_NONE);
    mDFBSurface->Blit(mDFBSurface, img->getDFBSurface(), NULL, x, y);

    if(res)
        res->setRectangle(x, y, img->getSize());
}

void Window::drawImageStretch(Image* img, Rectangle& dest, Rectangle* res)
{
    DFBRectangle dfbRect;

    if(mDFBSurface == NULL)
        return;

    if(img == NULL)
        return;

    dfbRect.x = dest.getX();
    dfbRect.y = dest.getY();
    dfbRect.w = dest.getWidth();
    dfbRect.h = dest.getHeight();

    mDFBSurface->SetBlittingFlags(mDFBSurface, DSBLIT_BLEND_ALPHACHANNEL);
    //mDFBSurface->SetPorterDuff(mDFBSurface, DSPD_SRC_OVER);
    mDFBSurface->SetPorterDuff(mDFBSurface, DSPD_NONE);
    mDFBSurface->StretchBlit(mDFBSurface, img->getDFBSurface(), NULL, &dfbRect);

    if(res != NULL)
        res->setRectangle(dfbRect.x, dfbRect.y, dfbRect.w, dfbRect.h);
}

void Window::drawImageStretchFixed(Image* img, Rectangle& dest, Rectangle* res)
{
    DFBRectangle dfbRect;

    if(mDFBSurface == NULL)
        return;

    if(img == NULL)
        return;

    float scale;
    float scaleW = (float)dest.getWidth() / img->getWidth();
    float scaleH = (float)dest.getHeight() / img->getHeight();

    scale = (scaleW < scaleH) ? scaleW : scaleH;

    LOG_DEBUG("==== image : %d %d \n", img->getWidth(), img->getHeight());
    LOG_DEBUG("==== scale : %f, %d %d \n", scale, dest.getWidth(), dest.getHeight());
    
    dfbRect.w = img->getWidth() * scale;
    dfbRect.h = img->getHeight() * scale;

    dfbRect.x = dest.getX() + (dest.getWidth() - dfbRect.w)/ 2;
    dfbRect.y = dest.getY() + (dest.getHeight() - dfbRect.h)/ 2;
    
    LOG_INFO("==== %d, %d, %d X %d\n", dfbRect.x, dfbRect.y, dfbRect.w, dfbRect.h);
    
    mDFBSurface->SetBlittingFlags(mDFBSurface, DSBLIT_BLEND_ALPHACHANNEL);
    //mDFBSurface->SetPorterDuff(mDFBSurface, DSPD_SRC_OVER);
    mDFBSurface->SetPorterDuff(mDFBSurface, DSPD_NONE);
    mDFBSurface->StretchBlit(mDFBSurface, img->getDFBSurface(), NULL, &dfbRect);

    if(res != NULL)
        res->setRectangle(dfbRect.x, dfbRect.y, dfbRect.w, dfbRect.h);
}

void Window::drawRectangle(const Rectangle& rect, const Color& color)
{
    if(mDFBSurface == NULL)
        return;

    mDFBSurface->SetColor(mDFBSurface, color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
    mDFBSurface->SetBlittingFlags(mDFBSurface, DSBLIT_BLEND_ALPHACHANNEL);
    mDFBSurface->FillRectangle(mDFBSurface, rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
}

void Window::clear(const Color& color)
{
    if(mDFBSurface == NULL)
        return;

    mDFBSurface->Clear(mDFBSurface, color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha());
}

void Window::clear(const Rectangle& rect, const Color& color)
{
    if(mDFBSurface == NULL)
        return;

    drawRectangle(rect, color);
}

void Window::update()
{
    if(mDFBSurface == NULL)
        return;

    //mDFBSurface->Flip(mDFBSurface, NULL, DSFLIP_NONE);
    mDFBSurface->Flip(mDFBSurface, NULL, DSFLIP_WAITFORSYNC);
}

void Window::update(const Rectangle& rect)
{
    if(mDFBSurface == NULL)
        return;

    DFBRegion region;
    region.x1 = rect.getX();
    region.y1 = rect.getY();
    region.x2 = rect.getX() + rect.getWidth() - 1;
    region.y2 = rect.getY() + rect.getHeight() - 1;
        
    //mDFBSurface->Flip(mDFBSurface, &region, DSFLIP_NONE);
    mDFBSurface->Flip(mDFBSurface, &region, DSFLIP_WAITFORSYNC);
}

void Window::moveTo(int x, int y)
{
    /* TBD. check gard */
    mDFBWindow->MoveTo(mDFBWindow, x, y);
}

bool Window::isVisible()
{
    u8 opacity;

    if(mDFBWindow == NULL)
        return false;

    if(mDFBWindow->GetOpacity(mDFBWindow, &opacity) != DFB_OK)
        return false;

    return (opacity != 0);
}

void Window::show()
{
    if(mDFBWindow == NULL)
        return;

    if(isVisible())
        return;

    mDFBWindow->RaiseToTop(mDFBWindow);

    mDFBWindow->SetOpacity(mDFBWindow, mOpacity);
}

void Window::hide()
{
    if(mDFBWindow == NULL)
        return;

    mDFBWindow->SetOpacity(mDFBWindow, 0);
}

bool Window::init()
{
    DFBResult ret;

    Size size = DFBManager::getInstance().getLayerSize();

    DFBWindowDescription desc;
    desc.flags  = (DFBWindowDescriptionFlags)(DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_CAPS | DWDESC_OPTIONS | DWDESC_STACKING | DWDESC_SURFACE_CAPS);
    //desc.caps   = (DFBWindowCapabilities) (DWCAPS_DOUBLEBUFFER | DWCAPS_ALPHACHANNEL);
    desc.caps   = (DFBWindowCapabilities) DWCAPS_ALPHACHANNEL;
    desc.options = (DFBWindowOptions) (DWOP_ALPHACHANNEL);
    desc.surface_caps = (DFBSurfaceCapabilities)(DSCAPS_PREMULTIPLIED | DSCAPS_DOUBLE);

    switch(mOsdLayer)
    {
        case OSD_LAYER_BUTTOM:
            desc.stacking =  DWSC_LOWER;
            break;
        case OSD_LAYER_MIDDLE:
            desc.stacking = DWSC_MIDDLE;
            break;
        case OSD_LAYER_TOP:
            desc.stacking = DWSC_UPPER;
            break;
        default:
            desc.stacking = DWSC_UPPER;
            break;
    }

    desc.posx   = mX;
    desc.posy   = mY;
    desc.width  = mWidth;
    desc.height = mHeight;

    ret = DFBManager::getInstance().getLayer()->CreateWindow(DFBManager::getInstance().getLayer(), &desc, &mDFBWindow);;
    if(ret != DFB_OK)
    {
        LOG_ERROR("cannot create window : %s\n", DirectFBErrorString(ret));
        return false;
    }

    ret = mDFBWindow->GetSurface(mDFBWindow, &mDFBSurface);
    if(ret != DFB_OK)
    {
        LOG_ERROR("cannot acqure surface \n");
        return false;
    }

    if (mListener != NULL)
        mListener->onCreateWindow(this);

    return true;
}


void Window::deinit()
{
    if (mListener != NULL)
        mListener->onDestroyWindow(this);

    if(mDFBWindow)
    {
        // TODO. IMPLEMENTS HERE
    }
}

} // namespace PrazenUI
