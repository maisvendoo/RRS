#ifndef FILMSERVER_H
#define FILMSERVER_H


#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QDataStream>



struct tcp_config_film_t
{
    quint16 port;
    int data_send_interval; // интервал оправки сообщений

    tcp_config_film_t()
        : port(1995)
        , data_send_interval(100)
    {

    }
};





//  Структура данных общения с клиентом!
#pragma pack(push, 1)
struct data_client_t
{
    QString routeName;          ///< Имя маршрута
    float   trainCoordinate;    ///< Координата поезда
    float   trainVelocity;      ///< Скорость поезда

    data_client_t()
        : routeName("S-ZB")
        , trainCoordinate(0.0)
        , trainVelocity(0.0)
    {

    }

    // Метод для перевода данных в последовательность байт,
    // необходимый для отправки данных Клиенту!
    QByteArray serialize()
    {
        QByteArray data_out;
        QDataStream data_s(&data_out, QIODevice::WriteOnly);

        data_s << routeName
               << trainCoordinate
               << trainVelocity;

        return data_out;
    }
};
#pragma pack(pop)



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
class FilmServer : public QTcpServer
{
    Q_OBJECT

public:
    FilmServer(QObject *parent = Q_NULLPTR);

    void start();


public slots:
    // Отправка данных клиенту
    void slotSendDataClient(data_client_t data);


private:
    QTimer *timerSend_;

    QTcpSocket *client_;

    tcp_config_film_t tcp_conf_;

    void loadCfg_();


private slots:
    void slotClientAutorized();
    void slotClientDisconnected();



};

#endif // FILMSERVER_H
