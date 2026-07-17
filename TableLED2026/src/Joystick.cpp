

#include "Arduino.h"
#include "Joystick.h"

void Joystick::Begin()
{
    pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
}
int16_t Joystick::GetX()
{
    return 4095-analogRead(JOYSTICK_X_PIN)-2047;
}
int16_t Joystick::GetY()
{
    return analogRead(JOYSTICK_Y_PIN)-2047;
}
bool Joystick::GetDown()
{
    return digitalRead(JOYSTICK_BUTTON_PIN) == 0;
}
