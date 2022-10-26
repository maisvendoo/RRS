#include    "registrator.h"

#include    "filesystem.h"

#include    <QDir>
#include    <QFile>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Registrator::Registrator(QString log_name, double timeout)
    : log_name(log_name)
    , timeout(timeout)
    , first_msg(true)
    , tau(0)
{
    FileSystem &fs = FileSystem::getInstance();
    logs_dir = QString(fs.getLogsDir().c_str());
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
void Registrator::print_msg(QString msg, double t, double dt)
{
    QString path = logs_dir + QDir::separator() + log_name;
    QFile file(path);

    if (first_msg || (tau >= timeout))
    {
        if (file.open(QIODevice::Text | QIODevice::Append))
        {
            first_msg = false;
            tau = 0;
            QTextStream s(&file);

            s << QString("%1 ").arg(t) << msg << "\n";

            file.close();
        }
    }

    tau += dt;
}


