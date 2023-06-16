#include    "tep70.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TEP70::stepDecodeAlsn()
{
    analogSignal[LS_W] = 0.0f;
    analogSignal[LS_YR] = 0.0f;
    analogSignal[LS_R] = 0.0f;
    analogSignal[LS_Y] = 0.0f;
    analogSignal[LS_G] = 0.0f;

    if (!epk->getStateKey())
        return;

    switch (alsn_info.code_alsn)
    {
    case ALSN_NOCODE:

        analogSignal[LS_W] = 1.0f;

        break;

    case ALSN_RED_YELLOW:

        analogSignal[LS_YR] = 1.0f;

        break;

    case ALSN_YELLOW:

        analogSignal[LS_Y] = 1.0f;

        break;

    case ALSN_GREEN:

        analogSignal[LS_G] = 1.0f;

        break;
    }
}
