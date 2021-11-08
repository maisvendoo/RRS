#include    "chs2t.h"
#include    "chs2t-signals.h"

void CHS2T::stepDecodeAlsn()
{
    analogSignal[LS_W] = safety_device->getWhiteLamp();
    analogSignal[LS_YR] = safety_device->getRedYellowLamp();
    analogSignal[LS_R] = safety_device->getRedLamp();
    analogSignal[LS_Y] = safety_device->getYellowLamp();
    analogSignal[LS_G] = safety_device->getGreenLamp();
}
