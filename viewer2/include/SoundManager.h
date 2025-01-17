#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <alc.h>
class SoundManager
{
public:
    SoundManager();
    ~SoundManager();

private:
    void init();

    ALCdevice* device;
    ALCcontext* context;
};

#endif // SOUND_MANAGER_H
