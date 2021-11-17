#include    "vl60.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::stepDecodeAlsn()
{
    analogSignal[LS_W] = 0.0f;
    analogSignal[LS_YK] = 0.0f;
    analogSignal[LS_R] = 0.0f;
    analogSignal[LS_Y] = 0.0f;
    analogSignal[LS_G] = 0.0f;

    switch (alsn_info.code_alsn)
    {
    case ALSN_NOCODE:

        analogSignal[LS_W] = 1.0f;

        break;

    case ALSN_RED_YELLOW:

        analogSignal[LS_YK] = 1.0f;

        break;

    case ALSN_YELLOW:

        analogSignal[LS_Y] = 1.0f;

        break;

    case ALSN_GREEN:

        analogSignal[LS_G] = 1.0f;

        break;
    }
}
