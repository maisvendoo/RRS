#ifndef CHS2T_AUTOSTOP_H
#define CHS2T_AUTOSTOP_H

#include "automatic-train-stop.h"

class CHS2TAutostop : public AutoTrainStop
{
public:

    /*CHS2TAutostop(QObject *parent = Q_NULLPTR)
        : AutoTrainStop(parent)
    {

    }*/

    //~CHS2TAutostop() {}

    bool getState()
    {
        return is_key_on;
    }
};

#endif // CHS2T_AUTOSTOP_H
