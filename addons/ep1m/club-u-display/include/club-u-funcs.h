#ifndef CLUBUFUNCS_H
#define CLUBUFUNCS_H

#include    <QString>
#include    <QDir>

QString getConfigPath(QString config_name)
{
    return QDir::separator() + QString("CLUB-U") + QDir::separator() + config_name;
}

#endif // CLUBUFUNCS_H
