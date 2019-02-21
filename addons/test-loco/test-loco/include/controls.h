#ifndef CONTROLS_H
#define CONTROLS_H

#include    "trigger.h"

class IncTractionTrigger : public Trigger<double>
{
private:

    void operate(double &value);
};

class DecTractionTrigger : public Trigger<double>
{
private:

    void operate(double &value);
};

class IncBrakeCrane : public Trigger<int>
{
private:

    void operate(int &value);
};

class DecBrakeCrane : public Trigger<int>
{
private:

    void operate(int &value);
};

class IncChargePress : public Trigger<double>
{
private:

    void operate(double &value)
    {
        value += 0.05;
    }
};

class DecChargePress : public Trigger<double>
{
private:

    void operate(double &value)
    {
        value -= 0.05;
    }
};

#endif // CONTROLS_H
