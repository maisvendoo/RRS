#include    "chs2t-horn.h"

CHS2tHorn::CHS2tHorn(QObject *parent) : TrainHorn (parent)

{

}

CHS2tHorn::~CHS2tHorn()
{

}

void CHS2tHorn::stepExternalControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    bool is_svistok_old = is_svistok;
    is_svistok = static_cast<bool>(control_signals.analogSignal[KM_SVISTOK].cur_value);

    if (is_svistok_old != is_svistok)

    {
        if (is_svistok)
        {
            emit soundPlay("Svistok");
        }
        else
        {
            emit soundStop("Svistok");
        }
    }
}


