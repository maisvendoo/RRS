#ifndef TRAIN_EXTERIOR_HANDLER_H
#define TRAIN_EXTERIOR_HANDLER_H

#include <memory>

class SoundManager;

struct settings_t;

class TrainExteriorHandler
{
public:
    TrainExteriorHandler(const settings_t& settings, const std::unique_ptr<SoundManager>& sm);

private:
    double settings_delay;
};

#endif // TRAIN_EXTERIOR_HANDLER_H
