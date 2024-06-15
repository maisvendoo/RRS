#include    "registrator.h"
#include    "filesystem.h"
#include    "Journal.h"

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
    custom_cfg_dir = "";
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
void Registrator::setCustomConfigDir(const QString &path)
{
    custom_cfg_dir = path;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::read_config(const QString &filename, const QString &dir_path)
{
    FileSystem &fs = FileSystem::getInstance();
    CfgReader cfg;

    // Custom config from path
    if (dir_path != "")
    {
        QString cfg_path = dir_path + QDir::separator() + filename + ".xml";

        if (cfg.load(cfg_path))
        {
            Journal::instance()->info("Loaded file: " + cfg_path);

            load_configuration(cfg);
            load_config(cfg);
            return;
        }
        else
        {
            Journal::instance()->error("File " + filename + ".xml is't found at custom path " + dir_path);
        }
    }

    // Custom config from vehicle's subdirectory
    if (custom_cfg_dir != "")
    {
        QString cfg_path = custom_cfg_dir + QDir::separator() + filename + ".xml";

        if (cfg.load(cfg_path))
        {
            Journal::instance()->info("Loaded file: " + cfg_path);

            load_configuration(cfg);
            load_config(cfg);
            return;
        }
        else
        {
            Journal::instance()->error("File " + filename + ".xml is't found at custom path " + custom_cfg_dir);
        }
    }

    // Config from default directory
    QString cfg_dir = fs.getDevicesDir().c_str();
    QString cfg_path = cfg_dir + QDir::separator() + filename + ".xml";

    if (cfg.load(cfg_path))
    {
        Journal::instance()->info("Loaded file: " + cfg_path);

        load_configuration(cfg);
        load_config(cfg);
        return;
    }
    Journal::instance()->error("File " + filename + ".xml is't found at default path " + cfg_dir);
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
void Registrator::load_configuration(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "Interval", interval);
    tau = interval;

    cfg.getString(secName, "FileName", fileName);
    cfg.getBool(secName, "ReplaceDotByComma", is_replace_dot_by_comma);
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Registrator::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}
