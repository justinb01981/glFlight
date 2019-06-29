#pragma once

#include <map>
#include "AudioFile.h"

#ifdef INCLUDE_VIA_GAMEAUDIO_CPP
#include "AudioFile.cpp"
#endif

#define NUM_BUFFERS 64

typedef float SampleType;
typedef short ALSampleType;

extern AudioFile<SampleType> files[NUM_BUFFERS];
extern unsigned int source[NUM_BUFFERS];
extern std::map<std::string, unsigned int> sourceMap;

void openALPlay(const char* name, float vol, float rate);

int openALInit();

void openALUninit();