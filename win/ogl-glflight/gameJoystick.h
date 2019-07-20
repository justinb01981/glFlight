#pragma once
#include <GLFW/glfw3.h>

class gameJoystick
{
public:
    int KEY_ROLL_ADD = GLFW_KEY_Q;
    int KEY_ROLL_SUB = GLFW_KEY_E;
    int KEY_FIRE_LASER = GLFW_KEY_SPACE;
    int KEY_FIRE_MISSLE = GLFW_KEY_LEFT_SHIFT;
    int KEY_ACCEL = GLFW_KEY_W;
    int KEY_DECEL = GLFW_KEY_S;
    int JAXIS_ROLL = 4;
    int JAXIS_PITCH = 1;
    int JAXIS_YAW = 0;
    int JKEY_ACCEL = 4;
    int JKEY_DECEL = 5;
    int JKEY_FIRE_LASER = 2;
    int JKEY_FIRE_MISSLE = 3;

    int jD = 5;

public:
    gameJoystick();
    ~gameJoystick();
};

