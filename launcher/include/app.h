//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     APP_H
#define     APP_H

#include    <QApplication>

#include    "launcher-command-line.h"
#include    "launcher-config.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LauncherApp : public QApplication
{
    Q_OBJECT

public:

    explicit LauncherApp(int argc, char **argv);
    virtual ~LauncherApp();

    launcher_config_t getConfig() const;

private:

    launcher_config_t   launcher_config;
};

#endif // APP_H
