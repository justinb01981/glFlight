
#include "gameWinStubs.h"
#include <stdint.h> // portable: uint64_t   MSVC: __int64 
#include <Windows.h>

extern void angle_usleep(unsigned long);

void usleep(unsigned int usec)
{
    Sleep(usec / 1000);
}

int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}

void appWriteSettings()
{
}

static char settingsPath[255];

glFlightGameResourceInfo_t glFlightGameResourceInfo;

const char*
glFlightSettingsPath()
{
    sprintf(settingsPath, "%ssettings.txt", glFlightGameResourceInfo.pathPrefix);
    return settingsPath;
}

const char*
glFlightDefaultGameName()
{
    static char glFlightDefaultGameName_[256];

    sprintf(glFlightDefaultGameName_, "android%d", rand() % 1024);
    return glFlightDefaultGameName_;
}

const char*
glFlightDefaultPlayerName()
{
    static char glFlightDefaultPlayerName_[256];

    sprintf(glFlightDefaultPlayerName_, "player%d", rand() % 1024);
    return glFlightDefaultPlayerName_;
}

// https://stackoverflow.com/questions/12933309/linker-error-unresolved-external-symbol-imp-iob-func-in-libpng-lib
void iob_func(void* p) {

}