#ifndef __KEY_MAP_H_
#define __KEY_MAP_H_

#include "keycode.h"

#define UNKNOWN_VERSION    (-1)
#define UNKNOWN_VENDOR     (-1)
#define UNKNOWN_PRODUCT    (-1)
#define UNKNOWN_NAME       (NULL)

typedef struct InputDeviceInfo_s
{
    int mVersion;
    int mVendor;
    int mProduct;
    const char* mName;

    InputDevice_e eType;
} InputDeviceInfo_t;

InputDevice_e get_input_device_type(int version, int vendor, int product, const char* name);
int convert_key_code_from_map(int inputDevice, int rawKeyCode);

#endif /* __KEY_MAP_H_ */
