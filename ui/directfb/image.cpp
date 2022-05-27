#include "image.h"

#include "dfb_manager.h"
#include "rectangle.h"


Image::Image(const std::string& path, bool preload)
          : mDFBSurface(NULL),
            mPath(path),
            mWidth(-1),
            mHeight(-1)
{
    if(preload)
        loadImage();
}

Image::Image(const std::string& path, int width, int height, bool preload)
          : mDFBSurface(NULL),
            mPath(path),
            mWidth(width),
            mHeight(height)
{
    if(preload)
        loadImage();
}

Image::Image(Image& img)
          : mDFBSurface(NULL),
            mPath(img.mPath),
            mWidth(img.getWidth()),
            mHeight(img.getHeight())
{
}

Image::Image(Image* img, const Rectangle& rect)
          : mDFBSurface(NULL),
            mPath(""),
            mWidth(rect.getWidth()),
            mHeight(rect.getHeight())
{
    IDirectFBSurface* src = img->getDFBSurface();

    DFBRectangle srcRect;
    srcRect.x = rect.getX();
    srcRect.y = rect.getY();        
    srcRect.w = rect.getWidth();
    srcRect.h = rect.getHeight();

    src->GetSubSurface(src, &srcRect, &mDFBSurface);
}

Image::~Image()
{
    flush();
}


IDirectFBSurface* Image::getDFBSurface()
{
    if(!mDFBSurface)
        loadImage();
    
    return mDFBSurface;
}


int Image::getWidth()
{
    if(!mDFBSurface)
        loadImage();
    
    return mWidth;
}


int Image::getHeight()
{
    if(!mDFBSurface)
        loadImage();
    
    return mHeight;
}


Size Image::getSize()
{
    if(!mDFBSurface)
        loadImage();
    
    return Size(mWidth, mHeight);
}

void Image::flush()
{
    if(mDFBSurface != NULL)
    {
        mDFBSurface->Release(mDFBSurface);
        mDFBSurface = NULL;
    }
}

bool Image::loadImage()
{
    if(mDFBSurface)
        return true;

    DFBSurfaceDescription desc;
    
    IDirectFBImageProvider* provider;

    DFBResult ret = DFBManager::getInstance().getDFB()->CreateImageProvider(DFBManager::getInstance().getDFB(), mPath.c_str(), &provider);
    if(ret != DFB_OK)
    {
        LOG_ERROR("cannot create IDirectFBImageProvider (%s)\n", mPath.c_str());
        return false;
    }

    if(provider->GetSurfaceDescription(provider, &desc) != DFB_OK)
        LOG_ERROR("cannot GetSurfaceDescription\n");

    desc.flags = (DFBSurfaceDescriptionFlags) (desc.flags | DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT| DWDESC_PIXELFORMAT);
    desc.caps = DSCAPS_PREMULTIPLIED;
    desc.pixelformat = DSPF_ARGB;
    //desc.caps = DSCAPS_NONE;

    if(mWidth > 0)
        desc.width = mWidth;
    else
        mWidth = desc.width;

    if(mHeight > 0)
        desc.height = mHeight;
    else
        mHeight = desc.height;
        
    ret = DFBManager::getInstance().getDFB()->CreateSurface(DFBManager::getInstance().getDFB(), &desc, &mDFBSurface);
    if(ret != DFB_OK)
    {
        LOG_ERROR("cannot create surface \n");
        return false;
    }
    
    provider->RenderTo(provider, mDFBSurface, NULL);
    if(ret != DFB_OK)
        LOG_ERROR("cannot render to surface !\n");

    if(provider != NULL)
        provider->Release(provider);

    return true;
}
