#ifndef     LONG_FORCES_REGISTRATOR_H
#define     LONG_FORCES_REGISTRATOR_H

#include    <vector>

#include    <QFile>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LongForcesRegistrator
{
public:

    LongForcesRegistrator(double time_step = 0.01);

    ~LongForcesRegistrator();

    void print(const std::vector<double> &forces,
               const std::vector<double> &ds,
               double t,
               double dt);

private:

    bool first_value;

    double tau;

    double time_step;

    QFile *file;

    QString fileName;
};

#endif // LONG_FORCES_REGISTRATOR_H
