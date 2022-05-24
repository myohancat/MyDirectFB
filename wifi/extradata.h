#ifndef __EXTRA_DATA_H_
#define __EXTRA_DATA_H_

#include <string>
#include <wifi_device.h>
#include <wifi_p2p_device.h>
#include <wifi_p2p_group.h>
#include <wifi_p2p_config.h>

#define DELETE_OBJECT(CLASS, OBJ)    do \
                                    { \
                                        CLASS* obj = (CLASS*)OBJ; \
                                        delete obj; \
                                    } while(0)

class ExtraData
{
public:
    typedef enum
    {
        TYPE_INT,
        TYPE_CSTR,   /* free */
        TYPE_STRUCT, /* free */

        TYPE_STRING,
        TYPE_WIFI_DEVICE,
        TYPE_WIFI_P2P_DEVICE,
        TYPE_WIFI_P2P_GROUP,
        TYPE_WIFI_P2P_CONFIG,

    } DataType_e;

    ExtraData(DataType_e type, void* obj)
    { 
        mType = type;
        mObj  = obj;
    }

    ~ExtraData() 
    {
        switch(mType)
        {
            case TYPE_INT:
                /* NOP */
                break;
            case TYPE_CSTR:
            case TYPE_STRUCT:
                free(mObj);
                break;
            case TYPE_STRING:
                DELETE_OBJECT(std::string, mObj);
                break;
            case TYPE_WIFI_DEVICE:
                DELETE_OBJECT(WifiDevice, mObj);
                break;
            case TYPE_WIFI_P2P_DEVICE:
                DELETE_OBJECT(WifiP2pDevice, mObj);
                break;
            case TYPE_WIFI_P2P_GROUP:
                DELETE_OBJECT(WifiP2pGroup, mObj);
                break;
            case TYPE_WIFI_P2P_CONFIG:
                DELETE_OBJECT(WifiP2pConfig, mObj);
                break;
        }
    }

    DataType_e type()
    {
        return mType;
    }

    void* object()
    {
        return mObj;
    }

private:
    DataType_e  mType;
    void*       mObj;
};

#endif /* __EXTRA_DATA_H_ */
