#ifndef __SIZE_H_
#define __SIZE_H_

namespace PrazenUI
{

class Size
{
public:
    Size();

    Size(const Size& size);
    
    Size(int width, int height);

    ~Size();

    int getWidth() const;
    
    int getHeight() const;

    void setWidth(int width);

    void setHeight(int height);

    Size&
    operator=(const Size &size);

    bool
    operator==(const Size &size) const;

    bool
    operator!=(const Size &size) const;

private:
    int mWidth;

    int mHeight;
};

inline int Size::getWidth() const
{
    return mWidth;
}

inline int Size::getHeight() const
{
    return mHeight;
}

} // namespace PrazenUI
#endif /* __SIZE_H_ */
