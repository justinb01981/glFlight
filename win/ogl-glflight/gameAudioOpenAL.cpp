#include <iostream>
#include <limits>
#include "al.h"
#include "alc.h"

#define INCLUDE_VIA_GAMEAUDIO_CPP
#include "gameAudioOpenAL.h"

ALCdevice* Device;
ALCcontext* Context;
ALCuint g_Buffers[NUM_BUFFERS];
AudioFile<SampleType> files[NUM_BUFFERS];
ALuint source[NUM_BUFFERS];
std::map<std::string, unsigned int> sourceMap;
std::map<std::string, unsigned int> sourceCopyMap;
std::vector<ALSampleType>* storage[NUM_BUFFERS];

static void DisplayALError(const char* fmt, ALenum error)
{
    std::cout << fmt << error;
}

void loadWAVFile(const char* filename, ALenum* format, ALvoid** data, ALsizei* size, ALsizei* freq, ALsizei* loop, AudioFile<SampleType>& af, std::vector<ALSampleType>** storage)
{
    std::string fullfilename = filename;
    fullfilename.append(".wav");
    af.load(fullfilename);

    *loop = 1;
    *freq = af.getSampleRate();

    switch (af.getNumChannels()) {
    case 1:
        *format = AL_FORMAT_MONO16;
        *size = af.getNumSamplesPerChannel() * sizeof(ALSampleType);
        break;
    case 2:
        *format = AL_FORMAT_STEREO16;
        *size = af.getNumSamplesPerChannel() * sizeof(ALSampleType)*2;
    }

    *storage = new std::vector<ALSampleType>();

    unsigned int off = 0;
    for (off = 0; off < af.getNumSamplesPerChannel(); off++)
    {
        ALSampleType v;
        v = af.samples[0][off] * SHRT_MAX;
        (*storage)->push_back(v);

        if (af.getNumChannels() > 1) {
            v = af.samples[1][off] * SHRT_MAX;
            (*storage)->push_back(v);
        }
    }

    *data = (*storage)->data();
}

int openALInit()
{
    ALenum error, format;
    ALvoid *data;
    ALsizei freq, loop, size;

    // Initialization 
    Device = alcOpenDevice(NULL); // select the "preferred device"   
    if (Device) {  
        Context=alcCreateContext(Device,NULL);  
        alcMakeContextCurrent(Context);  
    } 

    // Check for EAX 2.0 support g_bEAX = alIsExtensionPresent("EAX2.0"); 

    alGetError(); // clear error code 

    return 0;
}

void openALUninit()
{
    // Exit 
    Context = alcGetCurrentContext();
    Device = alcGetContextsDevice(Context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(Context);
    alcCloseDevice(Device);
    
    for (auto i = sourceMap.begin(); i != sourceMap.end(); i++)
    {
        alDeleteSources(1, &source[(*i).second]);
        alDeleteBuffers(1, &g_Buffers[(*i).second]);
    }
}

int openALLoadSource(const char* name)
{
    ALenum error, format;
    ALvoid* data;
    ALsizei freq, loop, size;

    unsigned int loadIndex = sourceMap.size();

    // Load
    loadWAVFile(name, &format, &data, &size, &freq, &loop, files[loadIndex], &storage[loadIndex]);
    if ((error = alGetError()) != AL_NO_ERROR) {
        DisplayALError("alutLoadWAVFile test.wav : ", error);
        alDeleteBuffers(NUM_BUFFERS, g_Buffers);
        return 0;
    }

    alGenBuffers(1, &g_Buffers[loadIndex]);

    // Copy test.wav data into AL Buffer 0 
    alBufferData(g_Buffers[loadIndex], format, data, size, freq);
    if ((error = alGetError()) != AL_NO_ERROR) {
        DisplayALError("alBufferData buffer 0 : ", error);
        alDeleteBuffers(NUM_BUFFERS, g_Buffers);
        return 0;
    }

    // Generate Sources 
    alGenSources(1, &source[loadIndex]);
    if ((error = alGetError()) != AL_NO_ERROR) {
        DisplayALError("alGenSources 1 : ", error);
        return 0;
    }

    // Attach buffer 0 to source 
    alSourcei(source[loadIndex], AL_BUFFER, g_Buffers[loadIndex]);
    if ((error = alGetError()) != AL_NO_ERROR) {
        DisplayALError("alSourcei AL_BUFFER: ", error);
    }

    sourceMap[name] = loadIndex;
}

void openALPlay(const char* name, float vol, float rate)
{
    ALint source_state;
    ALenum error;
    unsigned int index = 0;

    while (true) {

        if (sourceMap[std::string(name)] == NULL) {
            openALLoadSource(name);
        }

        unsigned int index = sourceMap[name];
        auto foundSource = source[index];

        alGetSourcei(foundSource, AL_SOURCE_STATE, &source_state);
        if (source_state == AL_PLAYING && index < 3) {
            std::cout << "source playing already:" << name << std::endl;
            return;
        }

        alSourcef(foundSource, AL_PITCH, rate);
        alSourcef(foundSource, AL_GAIN, vol);

        alSourcePlay(foundSource);

        if ((error = alGetError()) != AL_NO_ERROR) {
            DisplayALError("alSourcePlay: ", error);
        }

        break;
    }
}