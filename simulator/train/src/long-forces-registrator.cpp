#include    "long-forces-registrator.h"

#include    <QDir>
#include    <QDateTime>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
LongForcesRegistrator::LongForcesRegistrator(double time_step)
    : first_value(true)
    , tau(0.0)
    , time_step(time_step)
    , file(Q_NULLPTR)
{
    fileName = "../graphs/";
    fileName += "long_forces" +
            QDateTime::currentDateTime().toString("_dd-MM-yyyy_hh-mm-ss") +
            ".txt";

    file = new QFile(fileName);
    file->open(QIODevice::WriteOnly);
    file->close();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void LongForcesRegistrator::print(const std::vector<double> &forces,
                                  const std::vector<double> &ds,
                                  double t,
                                  double dt)
{
    if (forces.size() == 0)
        return;

    if ( first_value || (tau >= time_step) )
    {
        first_value = false;
        tau = 0.0;

        file->open(QIODevice::Append);

        QTextStream stream(file);

        stream << t;

        for (size_t i = 0; i < forces.size(); ++i)
        {
           stream << " " << ds[i] << " " << forces[i];
        }

        stream << "\n";
    }

    tau += dt;
}
