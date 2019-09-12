#include    "registrator.h"

#include    <QDir>
#include    <QDateTime>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Registrator::Registrator(QString fileName, double interval, QObject *parent)
    : QObject(parent)
    , first_print(true)
    , tau(0.0)
    , interval(interval)
    , path("")
{
    path = "../graphs/";
    path += fileName + QDateTime::currentDateTime().toString("_dd-MM-yyyy_hh-mm-ss") + ".txt";

    file = new QFile(path);
    file->open(QIODevice::WriteOnly);
    file->close();
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
    if ( first_print || (tau > interval))
    {
        file->open(QIODevice::Append);

        QTextStream stream(file);

        stream << line << "\n";

        file->close();
    }

    tau += dt;
}
