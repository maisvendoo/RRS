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

    virtual void read_custom_config(const QString &path);

    void setFileName(QString name);
    void setInterval(double interval);

    virtual void init();

    virtual void print(QString line, double t = 0.0, double dt = 0.0);

protected:

    double  tau;
    double  interval;
    QString fileName;
    QString path;

    QFile   *file;

    virtual void load_config(CfgReader &cfg);
};


#endif // REGISTRATOR_H
