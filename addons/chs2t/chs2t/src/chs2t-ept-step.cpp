#include    "chs2t.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CHS2T::stepEPT(double t, double dt)
{
    EPTControlLine *prev_line = nullptr;
    EPTControlLine *next_line = nullptr;

    if (prev_vehicle != nullptr)
    {
        prev_line = prev_vehicle->getEPTLine(0);
    }

    if (next_vehicle != nullptr)
    {
        next_line = next_vehicle->getEPTLine(0);
    }

    double U_prev = 0.0;

    if (prev_line != nullptr)
    {
        prev_line->setOutputCurrent(eptLine[0]->getI_in());
        U_prev = prev_line->getU_out();
    }

    eptLine[0]->setInputVoltage(max(U_prev, 55.0));

    double I_next = 0;

    if (next_line != nullptr)
    {
        I_next = next_line->getI_in();
    }

    eptLine[0]->setOutputCurrent(I_next);

    eptLine[0]->setValveState(0, electroAirDistr->getValveState(0));
    eptLine[0]->setValveState(1, electroAirDistr->getValveState(1));
    eptLine[0]->step(t, dt);
}
