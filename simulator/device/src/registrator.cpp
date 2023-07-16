#include    "registrator.h"

#include    <QDir>
#include    <QDateTime>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Registrator::Registrator(double interval, QObject *parent)
    : Device(parent)
    , first_print(true)
    , tau(0.0)
    , interval(interval)
    , fileName("data.txt")
    , path("")
    , file(Q_NULLPTR)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Registrator::~Registrator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::print(QString line, double t, double dt)
{
    Q_UNUSED(t)

    if ( first_print || (tau > interval))
    {
        file->open(QIODevice::Append);

        QTextStream stream(file);

        stream << line << "\n";

        file->close();
    }

    tau += dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::ode_system(const state_vector_t &Y,
                             state_vector_t &dYdt,
                             double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Interval", interval);
    cfg.getString(secName, "FileName", fileName);

    path = "../graphs/";
    path += fileName + QDateTime::currentDateTime().toString("_dd-MM-yyyy_hh-mm-ss") + ".txt";

    file = new QFile(path);
    file->open(QIODevice::WriteOnly);
    file->close();
}
