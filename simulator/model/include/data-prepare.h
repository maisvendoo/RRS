#ifndef     DATA_PREPARE_H
#define     DATA_PREPARE_H

#include    "abstract-data-engine.h"

class DataPrepare : public AbstractDataEngine
{
public:

    DataPrepare();

    QByteArray getPreparedData() Q_DECL_OVERRIDE;
};

#endif // DATA_PREPARE_H
