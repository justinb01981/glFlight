#include "gameJoystick.h"
#include <fstream>
#include <iostream>
#include <string>
#include <map>

using namespace std;

/*
    static int KEY_ROLL_ADD = GLFW_KEY_Q;
    static int KEY_ROLL_SUB = GLFW_KEY_E;
    static int KEY_FIRE_LASER = GLFW_KEY_SPACE;
    static int KEY_FIRE_MISSLE = GLFW_KEY_LEFT_SHIFT;
    static int KEY_ACCEL = GLFW_KEY_W;
    static int KEY_DECEL = GLFW_KEY_S;
    static int JAXIS_ROLL = 4;
    static int JAXIS_PITCH = 1;
    static int JAXIS_YAW = 0;
    static int JKEY_ACCEL = 4;
    static int JKEY_DECEL = 5;
    static int JKEY_FIRE_LASER = 2;
    static int JKEY_FIRE_MISSLE = 3;

    float jD = 5;
*/

gameJoystick::gameJoystick()
{
    fstream fjoyConfig = fstream("joy_config.txt", ios_base::in);
    string line = string(2048, '\0');
    struct kComp {
        bool operator()(char* a, char* b) const {
            return strcmp(a, b)<0;
        }
    };
    map<char*, int*, kComp> valueMap;

    valueMap["KEY_ROLL_ADD"] = &KEY_ROLL_ADD;
    valueMap["KEY_ROLL_SUB"] = &KEY_ROLL_SUB;
    valueMap["KEY_FIRE_LASER"] = &KEY_FIRE_LASER;
    valueMap["KEY_FIRE_MISSLE"] = &KEY_FIRE_MISSLE;
    valueMap["KEY_ACCEL"] = &KEY_ACCEL;
    valueMap["KEY_DECEL"] = &KEY_DECEL;
    valueMap["JAXIS_ROLL"] = &JAXIS_ROLL;
    valueMap["JAXIS_PITCH"] = &JAXIS_PITCH;
    valueMap["JAXIS_YAW"] = &JAXIS_YAW;
    valueMap["JAXIS_PITCH"] = &JAXIS_PITCH;
    valueMap["JKEY_ACCEL"] = &JKEY_ACCEL;
    valueMap["JKEY_DECEL"] = &JKEY_DECEL;
    valueMap["JKEY_FIRE_LASER"] = &JKEY_FIRE_LASER;
    valueMap["JKEY_FIRE_MISSLE"] = &JKEY_FIRE_MISSLE;
    valueMap["jD"] = &jD;

    if (!fjoyConfig.is_open()) {
        cout << "unable to open joy_config.txt" << endl;
        return;
    }

    while (!fjoyConfig.eof())
    {
        fjoyConfig.getline((char*)line.c_str(), line.capacity()-1);
        size_t off;

        off = line.find('=', 0);
        if (off > 0)
        {
            auto pre = line.substr(0, off);
            if (valueMap[(char*) pre.c_str()] != NULL)
            {
                int* dest = valueMap[(char*)pre.c_str()];
                char* ptr = (char*) line.c_str() + off + 1;
                if (sscanf(ptr, "%d", dest) < 1)
                {
                    cout << "unable to parse line: " << line.c_str() << endl;
                }
            }
        }
    }
}


gameJoystick::~gameJoystick()
{
}
