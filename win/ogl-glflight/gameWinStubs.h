#pragma once

typedef struct
{
    char pathPrefix[1024];
} glFlightGameResourceInfo_t;

extern glFlightGameResourceInfo_t glFlightGameResourceInfo;

void usleep(unsigned int usec);

unsigned long gettimeofday();

const char* glFlightSettingsPath();

const char* glFlightDefaultGameName();

const char* glFlightDefaultPlayerName();
