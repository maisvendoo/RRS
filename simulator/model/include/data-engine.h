#ifndef     DATA_ENGINE_H
#define     DATA_ENGINE_H

#include    "abstract-engine-definer.h"

class DataEngine : public AbstractEngineDefiner
{
public:

    DataEngine();

    AbstractDataEngine *getDataEngine_(QString name) Q_DECL_OVERRIDE;
};

#endif // DATA_ENGINE_H
