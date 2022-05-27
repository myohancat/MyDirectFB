#include "container.h"

#include <algorithm>
#include "dfb_manager.h"
#include "log.h"

static ContainerEvent  gContainerEvent;

ContainerEvent::ContainerEvent()
{
    int ret = pipe(mPipe);
    if(ret < 0)
    {
        LOG_ERROR("pipe failed (%d). %s\n", errno, strerror(errno));
        return;
    }

    EventLoop::getInstance().addFdWatcher(this);
}

ContainerEvent::~ContainerEvent()
{
    EventLoop::getInstance().removeFdWatcher(this);

    if(mPipe[0] >=0)
        close(mPipe[0]);
    if(mPipe[1] >= 0)
        close(mPipe[1]);
}

int ContainerEvent::post(int eventID, Container* container)
{
    int ret = write(mPipe[1], &eventID, sizeof(int));
    if(ret < 0)
    {
        LOG_ERROR("Cannot write event id \n");
        return ret;
    }

    ret = write(mPipe[1], &container, sizeof(container));
    if(ret < 0)
    {
        LOG_ERROR("Cannot write container \n");
        return ret;
    }

    return 0;
}

int ContainerEvent::postMessage(Container* container, int messageId, void* param)
{
    int eventId = CONTAINER_REUEST_MESSAGE;
    int ret = write(mPipe[1], &eventId, sizeof(int));
    if(ret < 0)
    {
        LOG_ERROR("Cannot write event id \n");
        return ret;
    }

    ret = write(mPipe[1], &container, sizeof(container));
    if(ret < 0)
    {
        LOG_ERROR("Cannot write container\n");
        return ret;
    }

    ret = write(mPipe[1], &messageId, sizeof(int));
    if(ret < 0)
    {
        LOG_ERROR("Cannot write message id \n");
        return ret;
    }

    ret = write(mPipe[1], &param, sizeof(void*));
    if(ret < 0)
    {
        LOG_ERROR("Cannot write param \n");
        return ret;
    }
    return 0;
}

int ContainerEvent::getFD()
{
    return mPipe[0];
}

bool ContainerEvent::onFdReadable(int fd)
{
    Container* container = NULL;
    int eventID = 0;

    int ret = read(fd, &eventID, sizeof(int));
    if(ret < 0)
    {
        LOG_ERROR("Cannot read event id \n");
        return true;
    }

    ret = read(fd, &container, sizeof(Container*));
    if(ret < 0)
    {
        LOG_ERROR("Cannot read event id \n");
        return true;
    }

    switch(eventID)
    {
        case CONTAINER_REUEST_SHOW:
        {
            if (container != NULL)
                container->_show();
            break;
        }
        case CONTAINER_REUEST_HIDE:
        {
            if (container != NULL)
                container->_hide();
            break;
        }
        case CONTAINER_REUEST_FOCUS:
        {
            if (container != NULL)
                container->setFocus(true);
            break;
        }
        case CONTAINER_REUEST_MESSAGE:
        {
            int   messageId;
            void* param;
            ret = read(fd, &messageId, sizeof(int));
            ret = read(fd, &param, sizeof(void*));

            if (container != NULL)
                container->onMessageReceived(messageId, param);
            break;
        }
        default:
            LOG_ERROR("Invalid Event ID : %d\n", eventID);
            break;
    }

    return true;
}

Container::Container(int osdLayer)
          : mContainerListener(NULL),
            mOsdLayer(osdLayer),
            mWindow(NULL),
            mBgImg(NULL),
            mBgColor(NULL)
{
    Size size = DFBManager::getInstance().getLayerSize();
    mX     = 0;
    mY     = 0;
    mWidth = size.getWidth();
    mHeight= size.getHeight();

    mFocused = false;
    mVisible = false;
}

Container::Container(int osdLayer, int width, int height)
          : mContainerListener(NULL),
            mOsdLayer(osdLayer),
            mWindow(NULL),
            mBgImg(NULL),
            mBgColor(NULL)
{
    Size size = DFBManager::getInstance().getLayerSize();
    mWidth = width;
    mHeight = height;

    if (mWidth < 0)
        mWidth = size.getWidth();
    if (mHeight < 0)
        mHeight = size.getHeight();

    mX = (size.getWidth() - mWidth) / 2;
    mY = (size.getHeight() - mHeight) / 2;

    mFocused = false;
    mVisible = false;
}

Container::Container(int osdLayer, int x, int y, int width, int height)
          : mContainerListener(NULL),
            mOsdLayer(osdLayer),
            mWindow(NULL),
            mBgImg(NULL),
            mBgColor(NULL)
{
    Size size = DFBManager::getInstance().getLayerSize();

    mX = x;
    mY = y;
    mWidth = width;
    mHeight = height;

    if (mWidth < 0)
        mWidth = size.getWidth();
    if (mHeight < 0)
        mHeight = size.getHeight();

    if (mX < 0)
        mX = (size.getWidth() - mWidth) / 2;
    if (mY < 0)
        mY = (size.getHeight() - mHeight) / 2;

    mFocused = false;
    mVisible = false;
}

Container::~Container()
{
    if(mBgImg != NULL)
        delete mBgImg;

    if(mBgImg != NULL)
        delete mBgColor;
    
    if(mWindow != NULL)
        delete mWindow;
}


void Container::setFocus(bool focus)
{
    if (mFocused == focus)
        return;

    onFocusChanged(focus);

    mFocused = focus;
}

// Override for focus
void Container::onFocusChanged(bool focus)
{
    // NOP
    UNUSED(focus);
}

void Container::addWidget(IWidget* widget)
{
    if (widget == NULL)
        return;

    WidgetList::iterator it = std::find(mWidgetList.begin(), mWidgetList.end(), widget);
    if(widget == *it)
        return;

    widget->setContainer(this);
    mWidgetList.push_back(widget);
}

void Container::removeWidget(IWidget* widget)
{
    if (widget == NULL)
        return;

    for(WidgetList::iterator it = mWidgetList.begin(); it != mWidgetList.end(); it++)
    {
        if(widget == *it)
        {
            mWidgetList.erase(it);
            (*it)->setContainer(NULL);
            return;
        }
    }
}

void Container::redrawWidgets()
{
    for(WidgetList::iterator it = mWidgetList.begin(); it != mWidgetList.end(); it++)
        (*it)->paint();
}

void Container::show()
{
    gContainerEvent.post(CONTAINER_REUEST_SHOW, this);
}

void Container::hide()
{
    gContainerEvent.post(CONTAINER_REUEST_HIDE, this);
}

void Container::requestFocus()
{
    gContainerEvent.post(CONTAINER_REUEST_FOCUS, this);    
}

void Container::sendMessage(int id, void* param)
{
    gContainerEvent.postMessage(this, id, param);
}

void Container::onMessageReceived(int messageId, void* param)
{
    UNUSED(messageId);
    UNUSED(param);
}

void Container::onCreateWindow(Window* window)
{
    UNUSED(window);

    if(mContainerListener)
        mContainerListener->onCreate(this);
}

void Container::onDestroyWindow(Window* window)
{
    UNUSED(window);

    if(mContainerListener)
        mContainerListener->onDestroy(this);
}

void Container::_show()
{
    if (mVisible == true)
    {
        LOG_WARN("already show.. skip it !\n");
        return;
    }

    // Lazy Creation
    if(mWindow == NULL)
    {
        mWindow = new Window(mOsdLayer, mX, mY, mWidth, mHeight, this);
        mWindow->clear();

        if(mContainerListener)
            mContainerListener->onCreate(this);

        for(WidgetList::iterator it = mWidgetList.begin(); it != mWidgetList.end(); it++)
            (*it)->paint();

        update();
    }

    if(mContainerListener)
        mContainerListener->onShow(this);

    if(mWindow != NULL)
        mWindow->show();

    mVisible = true;
}

void Container::_hide()
{
    if (mVisible == false)
    {
        LOG_WARN("already hide.. skip it !\n");
        return;
    }

    if(mContainerListener)
        mContainerListener->onHide(this);

    mVisible = false;

    if(mWindow == NULL)
        return;
    
    if(!mWindow->isVisible())
        return;

    mWindow->hide();

#if 1
    delete mWindow;
    mWindow = NULL;
#endif
}

void Container::setBackground(const Color& color, bool isUpdate)
{
    if(mBgImg != NULL)
    {
        delete mBgImg;
        mBgImg = NULL;
    }
    if(mBgColor != NULL)
    {
        delete mBgColor;
        mBgColor = NULL;
    }

    mBgColor = new Color(color);

    if(mWindow != NULL)
    {
        mWindow->clear(*mBgColor);

        if(isUpdate)
            mWindow->update();
    }
}

void Container::setBackground(const Image& img, bool isUpdate)
{
    if(mBgImg != NULL)
    {
        delete mBgImg;
        mBgImg = NULL;
    }
    if(mBgColor != NULL)
    {
        delete mBgColor;
        mBgColor = NULL;
    }

    mBgImg = new Image(img.getPath());

    if(mWindow != NULL)
    {
        mWindow->drawImage(mBgImg);

        if(isUpdate)
            mWindow->update();
    }
}

void Container::redrawBackground(const Rectangle& rect, bool isUpdate)
{
    if(mWindow == NULL)
        return;

    if(!rect.isValid())
        return;

    mWindow->clear(rect);
    if(mBgImg)
    {
        Image imgToRedraw(mBgImg, rect);
        mWindow->drawImage(&imgToRedraw, rect.getX(), rect.getY());
    }
    else if(mBgColor)
    {
        mWindow->drawRectangle(rect, *mBgColor);
    }

    if(isUpdate)
        mWindow->update(rect);
}

int Container::getScreenWidth() const
{
    if(!mWindow)
        return DFBManager::getInstance().getLayerSize().getWidth();

    return mWindow->getScreenWidth();
}

int Container::getScreenHeight() const
{
    if(!mWindow)
        return DFBManager::getInstance().getLayerSize().getHeight();

    return mWindow->getScreenHeight();
}
