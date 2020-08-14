#include    <QCoreApplication>
#include    <QTimer>

#include    "udp-client.h"

#include    "udp-server.h"

class Transmiter : public QObject
{
public:

    Transmiter(QObject *parent = Q_NULLPTR) : QObject(parent)
        , client(new UdpClient())
        , timer(new QTimer())
    {
        client->init("../cfg/udp-connection.xml");

        connect(timer, &QTimer::timeout, this, &Transmiter::slotSendRequest);
        connect(timer, &QTimer::timeout, this, &Transmiter::debug);

        timer->start(100);
    }

    ~Transmiter()
    {

    }

    void init() {}

private:
    UdpClient *client;

    QTimer *timer;

private slots:

    void slotSendRequest()
    {
        QString request = "request";
        QByteArray data;

        data = request.toUtf8();

        client->sendData(data);
    }

    void debug()
    {
        QString debug_info = QString("t = %1 msgCount = %2 vehicleCoord = %3 vehicleRPath = %4 \n")
                .arg(static_cast<double>(client->getDataTime()))
                .arg(client->getMsgCount())
                .arg(client->getVehicleCoord())
                .arg(client->getVehicleRPath());

        fputs(qPrintable(debug_info), stdout);
    }
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Transmiter *transmiter = new Transmiter;
    transmiter->init();

    return app.exec();
}
