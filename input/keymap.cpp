#include "keymap.h"

#include "keycode.h"
#include "types.h"

#include <string.h>

#include <linux/uinput.h>


static InputDeviceInfo_t INPUT_DEVICES[] =
{
    /* VERSION         VENDOR  PRODUCT   NAME                     DEVICE */
    { UNKNOWN_VERSION,   1,     1,     "adc-keys",           INPUT_DEVICE_ADC_KEYS  },
    { UNKNOWN_VERSION,   1,     1,     "gpio-keys",          INPUT_DEVICE_GPIO_KEYS },

    /* TODO. ADD DEVICE HERE */
};

InputDevice_e get_input_device_type(int version, int vendor, int product, const char* name)
{
    for (int ii = 0; ii < NELEM(INPUT_DEVICES); ii++)
    {
        InputDeviceInfo_t* device = &INPUT_DEVICES[ii];

        if (((device->mVersion == UNKNOWN_VERSION) || (device->mVersion == version))
         && ((device->mVendor  == UNKNOWN_VENDOR)  || (device->mVendor  == vendor))
         && ((device->mProduct == UNKNOWN_PRODUCT) || (device->mProduct == product))
         && ((device->mName    == UNKNOWN_NAME)    || strncmp(name, device->mName, strlen(device->mName)) == 0))
            return device->eType;
    }

    return INPUT_DEVICE_UNKNOWN;
}

typedef struct KeyMap_s
{
    int mRawKeyCode;
    int mKeyCode;
} KeyMap_t;


static KeyMap_t KEYMAP_DEFAULT[] =
{
    { KEY_BACK/* 158 */, KEYCODE_BACK }, /*이전*/

    { KEY_0 /* 11 */,    KEYCODE_0 },
    { KEY_1 /* 2 */ ,    KEYCODE_1 },
    { KEY_2 /* 3 */ ,    KEYCODE_2 },
    { KEY_3 /* 4 */ ,    KEYCODE_3 },
    { KEY_4 /* 5 */ ,    KEYCODE_4 },
    { KEY_5 /* 6 */ ,    KEYCODE_5 },
    { KEY_6 /* 7 */ ,    KEYCODE_6 },
    { KEY_7 /* 8 */ ,    KEYCODE_7 },
    { KEY_8 /* 9 */ ,    KEYCODE_8 },
    { KEY_9 /* 10 */,    KEYCODE_9 },

    { KEY_UP    /* 103 */, KEYCODE_DPAD_UP     },
    { KEY_DOWN  /* 108 */, KEYCODE_DPAD_DOWN   },
    { KEY_LEFT  /* 105 */, KEYCODE_DPAD_LEFT   },
    { KEY_RIGHT /* 106 */, KEYCODE_DPAD_RIGHT  },
    { KEY_ENTER /* 28  */, KEYCODE_DPAD_CENTER },

    { KEY_SLEEP/* 142 */,  KEYCODE_POWER       },

    { KEY_VOLUMEUP/* 115 */, KEYCODE_VOLUME_UP     },
    { KEY_VOLUMEDOWN/* 114 */, KEYCODE_VOLUME_DOWN },

    { KEY_PHONE/* 169 */, KEYCODE_MENU       }, /*옵션*/

    { KEY_CHANNELUP/* 0x192 */,   KEYCODE_CHANNEL_UP   },
    { KEY_CHANNELDOWN/* 0x193 */, KEYCODE_CHANNEL_DOWN },

    { KEY_ZOOMIN/* 0x1a2 */,   KEYCODE_ZOOM_IN   },
    { KEY_ZOOMOUT/* 0x1a3 */, KEYCODE_ZOOM_OUT   },

};

static KeyMap_t KEYMAP_GPIO_KEYS[] =
{
    { 116, KEYCODE_POWER        },
    { 114, KEYCODE_VOLUME_DOWN  },
    { 115, KEYCODE_VOLUME_UP    },
    { 418, KEYCODE_ZOOM_IN      },
    { 419, KEYCODE_ZOOM_OUT     },
};

int convert_key_code_from_map(int inputDevice, int rawKeyCode)
{
    KeyMap_t* keymap = NULL;
    int       size   = 0;

    switch(inputDevice)
    {
        case INPUT_DEVICE_GPIO_KEYS:
            keymap = KEYMAP_GPIO_KEYS;
            size   = NELEM(KEYMAP_GPIO_KEYS);
            break;
        case INPUT_DEVICE_ADC_KEYS:
        default:
            keymap = NULL;
            size   = 0;
            break;
    }

    /* 1. Convert Key code by Custom Key Map Table */
    for (int ii = 0; ii < size; ii++)
    {
        if (keymap[ii].mRawKeyCode == rawKeyCode)
            return keymap[ii].mKeyCode;
    }

    /* 2. Convert Key code by Default Key Map Table */
    for (int ii =0; ii < NELEM(KEYMAP_DEFAULT); ii++)
    {
        if (KEYMAP_DEFAULT[ii].mRawKeyCode == rawKeyCode)
            return KEYMAP_DEFAULT[ii].mKeyCode;
    }

    return KEYCODE_UNKNOWN;
}

