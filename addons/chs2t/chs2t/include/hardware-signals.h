#ifndef     HARDWARE_SIGNALS_H
#define     HARDWARE_SIGNALS_H

enum ControllerSignals
{
    KM_READY = 0,

    KM_K01 = 1,
    KM_K02 = 2,

    KM_K21 = 3,
    KM_K22 = 4,
    KM_K23 = 5,
    KM_K25 = 6,

    KM_K31 = 7,
    KM_K32 = 8,
    KM_K33 = 9,

    KM_SVISTOK = 10,
    KM_STURVAL_DOWN = 11,

    TEST_COUNT = 12
};

#endif // HARDWARESIGNALS_H
