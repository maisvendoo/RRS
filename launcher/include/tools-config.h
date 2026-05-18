#ifndef     TOOLS_CONFIG_H
#define     TOOLS_CONFIG_H

#include    <QString>
#include    <QStringList>
#include    <QProcess>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct tool_config_t
{
    QString     toolName = "";
    QString     workDir = "";
    QStringList args;
    QProcess    *proc = nullptr;
};

#endif
