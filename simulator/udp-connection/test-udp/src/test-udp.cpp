#include    <QCoreApplication>
#include    <QTimer>

#include    "udp-client.h"

/*class Foo : public QObject
{
    //Q_OBJECT

public:

    Foo();

    ~Foo();

private:

};*/



class Transmiter : public QObject
{


public:

    Transmiter(QObject *parent = Q_NULLPTR) : QObject(parent)
        , client(new UdpClient())
        , timer(new QTimer())
    {
        client->init("../cfg/udp-Sserver.xml");
        connect(timer, &QTimer::timeout, this, &Transmiter::slotSendRequest);

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

    //Foo *foo;
    //foo = new Foo();

    return app.exec();
}
