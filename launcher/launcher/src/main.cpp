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

    QObject::connect(osgWidget, &QtOSGWidget::startSimulation, &app, &LauncherApp::startSimulation);
    QObject::connect(osgWidget, &QtOSGWidget::stopSimulation, &app, &LauncherApp::stopSimulation);

    // TCP-clinet initialization
    TcpClient tcp_client;
    tcp_client_config_t tcp_client_config;
    tcp_client_config.address = launcher_config.host_address;
    tcp_client_config.port = launcher_config.port;
    tcp_client.init(tcp_client_config);

    QObject::connect(osgWidget, &QtOSGWidget::sendDataToSimulator,
                     &tcp_client, &TcpClient::sendDataToServer);

    tcp_client.start();

    // Check fullscreen mode
    if (launcher_config.fullscreen)
        window.showFullScreen();
    else
        window.show();

    // Start loop to signals processing
    return app.exec();
}
