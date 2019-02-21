#include    "controls.h"

void IncTractionTrigger::operate(double &value)
{
    value += 0.1;
}

void DecTractionTrigger::operate(double &value)
{
    value -= 0.1;
}

void IncBrakeCrane::operate(int &value)
{
    value++;
}

void DecBrakeCrane::operate(int &value)
{
    value--;
}
