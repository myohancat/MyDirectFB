#ifndef __DFB_MANAGER_H_
#define __DFB_MANAGER_H_

#include <directfb.h>
#include "log.h"
#include "size.h"

class DFBManager
{
public:
    static DFBManager& getInstance();

    IDirectFB*             getDFB() const;
    IDirectFBDisplayLayer* getLayer() const;
    
    Size getLayerSize();

private:
    IDirectFB* mDFB;
    IDirectFBDisplayLayer* mDFBLayer;
    IDirectFBSurface *     mDFBSurface;

    void init();
    void deinit();

    DFBManager();
    ~DFBManager();
};

#endif /* __DFB_MANAGER_H_ */
