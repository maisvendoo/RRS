#ifndef     CHS2T_HORN_H
#define     CHS2T_HORN_H

#include    "train-horn.h"
#include    "hardware-signals.h"

class CHS2tHorn : public TrainHorn
{
public:

    CHS2tHorn(QObject *parent = Q_NULLPTR);

    ~CHS2tHorn();

private:

    bool lock;

    void preStep(state_vector_t &Y, double t);
};

#endif // CHS2T_HORN_H
