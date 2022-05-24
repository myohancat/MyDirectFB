#include "dfb_manager.h"


namespace PrazenUI
{
    
DFBManager& DFBManager::getInstance()
{
    static DFBManager _instance;

    return _instance;
}

DFBManager::DFBManager()
        : mDFB(NULL),
          mDFBLayer(NULL)
{
    init();
}

DFBManager::~DFBManager()
{
    deinit();
}

IDirectFB* DFBManager::getDFB() const
{
    return mDFB;
}

IDirectFBDisplayLayer* DFBManager::getLayer() const
{
    return mDFBLayer;
}


void DFBManager::init()
{
#if 1
    char* settings[] = 
    {
        (char*)"ui",
        (char*)"--dfb:memcpy=asdk,no-cursor,system=fbdev,fbdev=/dev/fb0,no-vt,disable-module=linux_input",
    };
    
    int argc = sizeof(settings) / sizeof(settings[0]);
    char** argv = (char**)settings;

    if(DirectFBInit(&argc, &argv) != DFB_OK)
#else
    if(DirectFBInit(NULL, NULL) != DFB_OK)
#endif
    {
        LOG_ERROR("DirectFBInit() failed!\n");
        return;
    }

    if(DirectFBCreate(&mDFB) != DFB_OK)
    {
        LOG_ERROR("DirectFBCreate() failed!\n");
        return;
    }

    /* Create Layer */
    DFBDisplayLayerConfig config;

    if(mDFB->GetDisplayLayer(mDFB, DLID_PRIMARY, &mDFBLayer) != DFB_OK)
    {
        LOG_ERROR("GetDisplayLayer() failed!\n");
        return;
    }

    mDFBLayer->SetCooperativeLevel(mDFBLayer, DLSCL_ADMINISTRATIVE);

    mDFBLayer->GetConfiguration(mDFBLayer, &config);

    config.flags = (DFBDisplayLayerConfigFlags) (DLCONF_BUFFERMODE | DLCONF_OPTIONS);
    config.buffermode = DLBM_WINDOWS; /* DLBM_FRONTONLY, DLBM_BACKVIDEO, DLBM_BACKSYSTEM, DLBM_TRIPLE, DLBM_WINDOWS */
    mDFBLayer->SetConfiguration(mDFBLayer, &config);
}

Size DFBManager::getLayerSize()
{
    DFBDisplayLayerConfig conf;
    mDFBLayer->GetConfiguration(mDFBLayer, &conf);

    return Size(conf.width, conf.height);
}

void DFBManager::deinit()
{
    if(mDFB)
    {
        mDFB->Release(mDFB);
        mDFB = NULL;
    }
}

} /* namespace PrazenUI */
