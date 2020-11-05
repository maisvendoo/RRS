#include    "chs2t-horn.h"

CHS2tHorn::CHS2tHorn(QObject *parent) : TrainHorn (parent)
  , lock(false)
{

}

CHS2tHorn::~CHS2tHorn()
{

}

void CHS2tHorn::preStep(state_vector_t &Y, double t)
{
    if (control_signals.analogSignal[KM_SVISTOK].is_active)
    {
        is_svistok = static_cast<bool>(control_signals.analogSignal[KM_SVISTOK].cur_value);

        if (is_svistok && !lock)
        {
            emit soundPlay("Svistok");
            lock = true;
        }
        else
        {
            if (!is_svistok)
            {
                emit soundStop("Svistok");
                lock = false;
            }
        }
    }
}
