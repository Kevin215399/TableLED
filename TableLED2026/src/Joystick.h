#define JOYSTICK_X_PIN 39
#define JOYSTICK_Y_PIN 36
#define JOYSTICK_BUTTON_PIN 13

namespace Joystick {
    void Begin();
    //Range between -2047 to 2048
    int16_t GetX();
    //Range between -2047 to 2048
    int16_t GetY();
    bool GetDown();
}