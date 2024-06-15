#ifndef     REGISTRATOR_H
#define     REGISTRATOR_H

#include    <QObject>

#include    "device-export.h"
#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Registrator : public QObject
{

public:

    Registrator(double interval = 0.0, QObject *parent = Q_NULLPTR);

    ~Registrator();

    void setCustomConfigDir(const QString &path);

    /// Read device config file
    virtual void read_config(const QString &filename, const QString &dir_path);

    void setFileName(QString name);
    void setInterval(double interval);

    /// Автоматически заменять точки на запятые (для корректного импорта чисел в русифицированный Excel)
    void setReplaceDotByComma(bool value);

    virtual void init();

    virtual void print(QString line, double t = 0.0, double dt = 0.0);

protected:

    double  tau;
    double  interval;
    QString fileName;
    QString path;

    QFile   *file;

    bool is_replace_dot_by_comma;

    /// Path to directory with custom configs
    QString custom_cfg_dir;

    virtual void load_config(CfgReader &cfg);

private:

    void load_configuration(CfgReader &cfg);
};

#endif // REGISTRATOR_H
