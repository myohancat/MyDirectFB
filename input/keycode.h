#ifndef __KEY_CODE_
#define __KEY_CODE_

enum InputDevice_e
{
    INPUT_DEVICE_UNKNOWN = -1,

    INPUT_DEVICE_ADC_KEYS = 0,
    INPUT_DEVICE_GPIO_KEYS,

    /* TODO. ADD HERE */

    MAX_INPUT_DEVICE /* DO NOT MODIFY */
};

enum KeyState_e
{
    KEY_RELEASED,
    KEY_PRESSED,
    KEY_REPEATED,

    KEY_STATE_UNKNOWN /* DO NOT MODIFY */
};

enum KeyCode_e
{
    KEYCODE_UNKNOWN      = -1,

    KEYCODE_HOME         = 3,
    KEYCODE_BACK         = 4, /* 이전 */

    KEYCODE_0            = 7,
    KEYCODE_1            = 8,
    KEYCODE_2            = 9,
    KEYCODE_3            = 10,
    KEYCODE_4            = 11,
    KEYCODE_5            = 12,
    KEYCODE_6            = 13,
    KEYCODE_7            = 14,
    KEYCODE_8            = 15,
    KEYCODE_9            = 16,

    KEYCODE_DPAD_UP      = 19,
    KEYCODE_DPAD_DOWN    = 20,
    KEYCODE_DPAD_LEFT    = 21,
    KEYCODE_DPAD_RIGHT   = 22,
    KEYCODE_DPAD_CENTER  = 23,

    KEYCODE_VOLUME_UP    = 24,
    KEYCODE_VOLUME_DOWN  = 25,

    KEYCODE_POWER        = 26,

    KEYCODE_ENTER        = 66,

    KEYCODE_MENU         = 82, /* 옵션 */

    KEYCODE_CHANNEL_UP   = 166,
    KEYCODE_CHANNEL_DOWN = 167,

    KEYCODE_ZOOM_IN      = 168,
    KEYCODE_ZOOM_OUT     = 169,

    // TODO ADD KEYCODE HERE
};

#endif /* __KEY_CODE_ */
