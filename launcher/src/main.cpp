#include    "main.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    LauncherApp    app(argc, argv);

    QMainWindow window;

    launcher_config_t launcher_config = app.getConfig();
    window.setFixedWidth(launcher_config.width);
    window.setFixedHeight(launcher_config.height);

    QtOSGWidget *osgWidget = new QtOSGWidget(1.0f, 1.0f, &window);
    window.setCentralWidget(osgWidget);

    if (launcher_config.fullscreen)
        window.showFullScreen();
    else
        window.show();

    return app.exec();
}
