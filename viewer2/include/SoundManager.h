#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

struct ALCdevice;
struct ALCcontext;

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
