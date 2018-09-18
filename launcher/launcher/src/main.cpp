#include    "main.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Create application object
    LauncherApp    app(argc, argv);

    // Main window
    QMainWindow window;

    // Get application config
    launcher_config_t launcher_config = app.getConfig();

    // Windows size setting
    window.setFixedWidth(launcher_config.width);
    window.setFixedHeight(launcher_config.height);

    // OSG widget creation
    QtOSGWidget *osgWidget = new QtOSGWidget(1.0f, 1.0f, &window);
    window.setCentralWidget(osgWidget);

    // Check fullscreen mode
    if (launcher_config.fullscreen)
        window.showFullScreen();
    else
        window.show();

    // Start loop to signals processing
    return app.exec();
}
