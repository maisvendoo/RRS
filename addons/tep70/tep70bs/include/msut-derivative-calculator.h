#ifndef     MSUT_DERIVATIVE_CALCULATOR_H
#define     MSUT_DERIVATIVE_CALCULATOR_H

#include    <vector>

class DerivativeCalculator
{
public:

    DerivativeCalculator(double delta_t)
        : delta_t(delta_t)
        , derivative(0.0)
        , tau(0.0)
        , is_first_value(true)
    {

    }

    ~DerivativeCalculator()
    {

    }

    double getDerivative() const
    {
        return derivative;
    }

    void step(double x, double t, double dt)
    {
        if ((is_first_value) || (tau >= delta_t) )
        {
            is_first_value = false;
            tau = 0;

            values.push_back(x);

            if (values.size() > 3)
            {
                values.erase(values.begin());

                derivative = (3 * values[2] -
                              4 * values[1] +
                              values[0] ) / 2.0 / delta_t;
            }
        }

        tau += dt;
    }

private:

    double delta_t;

    double derivative;

    double tau;

    bool is_first_value;

    std::vector<double> values;
};

#endif // MSUT_DERIVATIVE_CALCULATOR_H
