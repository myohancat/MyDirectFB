#ifndef __IMAGE_H_
#define __IMAGE_H_

#include <string>
#include "directfb.h"
#include "rectangle.h"
#include "size.h"


class Image
{
public:
    Image(const std::string& path, bool preload = false);
    Image(const std::string& path, int width, int height, bool preload = false);
    Image(Image& img); 
    Image(Image* img, const Rectangle& rect); // Load Sub image

    ~Image();

    int getWidth();
    int getHeight();
    
    Size getSize();
    
    std::string getPath() const;
    
    IDirectFBSurface* getDFBSurface();

    void flush();
private:
    IDirectFBSurface* mDFBSurface;
    
    std::string      mPath;
    
    int              mWidth;
    int              mHeight;

    bool loadImage();
    void invalidateSurface();
};

inline std::string Image::getPath() const
{
    return mPath;
}

#endif /* __IMAGE_H_ */
