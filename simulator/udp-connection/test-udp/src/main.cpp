#include    <QCoreApplication>
#include    <QTimer>

#include    "udp-client.h"

class Transmiter : public QObject
{
    Q_OBJECT

public:

    Transmiter(QObject *parent = Q_NULLPTR) : QObject(parent)
        , client(new UdpClient)
        , timer(new QTimer)
    {
        client->init("../cfg/udp-server.xml");
        connect(timer, &QTimer::timeout, this, &Transmiter::slotSentRequest);

        timer->start(100);
    }

    ~Transmiter()
    {

    }

    void init() {}

private:

    UdpClient *client;

    QTimer *timer;

private:

    void slotSentRequest()
    {
        QByteArray request;
        request.append(1);

        client->sendData(request);
    }
};

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Transmiter *transmiter = new Transmiter;
    transmiter->init();

    return app.exec();
}
