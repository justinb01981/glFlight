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

const unsigned long GLM_VIEW_WIDTH = 1280 /*2560*/;
const unsigned long GLM_VIEW_HEIGHT = 720/*1440*/;
