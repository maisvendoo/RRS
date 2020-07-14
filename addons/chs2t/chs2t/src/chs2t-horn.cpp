#include    "chs2t-horn.h"

CHS2tHorn::CHS2tHorn(QObject *parent) : TrainHorn (parent)
{

}

CHS2tHorn::~CHS2tHorn()
{

}

void CHS2tHorn::stepKeysControl(double t, double dt)
{
    TrainHorn::stepKeysControl(t, dt);
}
