#ifndef     BLOK_FUNCS_H
#define     BLOK_FUNCS_H

#include    <QString>
#include    <QDir>

QString getConfigPath(QString config_name)
{
    return QDir::separator() + QString("BLOK") + QDir::separator() + config_name;
}

#endif // BLOCK_FUNCS_H
