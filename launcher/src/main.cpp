#include    "main.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    LauncherApp    app(argc, argv);

    QMainWindow window;
    QtOSGWidget *osgWidget = new QtOSGWidget(1.0f, 1.0f, &window);
    window.setCentralWidget(osgWidget);
    window.show();

    return app.exec();
}
