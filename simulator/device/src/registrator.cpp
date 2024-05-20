#include    "registrator.h"

#include    <QDir>
#include    <QDateTime>
#include    <QTextStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Registrator::Registrator(double interval, QObject *parent)
    : QObject(parent)
    , tau(interval)
    , interval(interval)
    , fileName("data")
    , path("")
    , file(nullptr)
    , is_replace_dot_by_comma(false)
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
void Registrator::setFileName(QString name)
{
    fileName = name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::setInterval(double value)
{
    interval = value;
    tau = interval;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::setReplaceDotByComma(bool value)
{
    is_replace_dot_by_comma = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::init()
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
void Registrator::read_custom_config(const QString &path)
{
    CfgReader cfg;

    if (cfg.load(path + ".xml"))
    {
        load_config(cfg);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::print(QString line, double t, double dt)
{
    Q_UNUSED(t)

    if ((tau >= interval) && (file))
    {
        file->open(QIODevice::Append);

        QTextStream stream(file);

        if (is_replace_dot_by_comma)
        {
            stream << line.replace('.',',') << "\n";
        }
        else
        {
            stream << line << "\n";
        }

        file->close();

        tau = 0.0;
    }

    tau += dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Interval", interval);
    tau = interval;

    cfg.getString(secName, "FileName", fileName);
    cfg.getBool(secName, "ReplaceDotByComma", is_replace_dot_by_comma);
}
