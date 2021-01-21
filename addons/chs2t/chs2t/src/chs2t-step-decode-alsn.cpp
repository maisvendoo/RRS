#include    "chs2t.h"
#include    "chs2t-signals.h"

void CHS2T::stepDecodeAlsn()
{
    analogSignal[LS_W] = 0.0f;
    analogSignal[LS_YR] = 0.0f;
    analogSignal[LS_R] = 0.0f;
    analogSignal[LS_Y] = 0.0f;
    analogSignal[LS_G] = 0.0f;

    float is_epk_on = static_cast<float>(autoTrainStop->getState());

    switch (alsn_info.code_alsn)
    {
    case ALSN_WHITE:

        analogSignal[LS_W] = 1.0f * is_epk_on;

        break;

    case ALSN_RED:

        analogSignal[LS_R] = 1.0f * is_epk_on;

        break;

    case ALSN_RED_YELLOW:

        analogSignal[LS_YR] = 1.0f * is_epk_on;

        break;

    case ALSN_YELLOW:

        analogSignal[LS_Y] = 1.0f * is_epk_on;

        break;

    case ALSN_GREEN:

        analogSignal[LS_G] = 1.0f * is_epk_on;

        break;
    }
}
