#ifndef     REGISTRATOR_H
#define     REGISTRATOR_H

#include    <QObject>
#include    <QString>
#include    <QFile>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Registrator : public QObject
{
    Q_OBJECT

public:

    Registrator(QString fileName, double interval = 0.1, QObject *parent = Q_NULLPTR);

    ~Registrator();

    void print(QString line, double t, double dt);

private:

    bool    first_print;
    double  tau;
    double  interval;
    QString path;

    QFile   *file;
};


#endif // REGISTRATOR_H
