#ifndef     SERVER_INFO_H
#define     SERVER_INFO_H

#include    <QStringList>
#define     SAVED_SERVERS_FILE std::string("saved-servers.xml")

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct server_info_t
{
    QString server_name = "Local";
    uint8_t ipv4_1 = 127;
    uint8_t ipv4_2 = 0;
    uint8_t ipv4_3 = 0;
    uint8_t ipv4_4 = 1;
    uint16_t ipv4_port = 1992;

    server_info_t()
    {

    }

    void setHostAddress(QString host_address)
    {
        QStringList tokens = host_address.split(":");
        if (tokens.size() > 1)
            ipv4_port = static_cast<uint16_t>(tokens[1].toInt());

        QString tmp = tokens[0];
        tokens.clear();
        tokens = tmp.split(".");

        if (tokens.size() < 4)
            return;
        ipv4_1 = static_cast<uint8_t>(tokens[0].toInt());
        ipv4_2 = static_cast<uint8_t>(tokens[1].toInt());
        ipv4_3 = static_cast<uint8_t>(tokens[2].toInt());
        ipv4_4 = static_cast<uint8_t>(tokens[3].toInt());
    }

    QString getHostAddress()
    {
        return QString("%1.%2.%3.%4").arg(ipv4_1).arg(ipv4_2).arg(ipv4_3).arg(ipv4_4);
    }

    QString getHostAddressAndPort()
    {
        return QString("%1.%2.%3.%4:%5").arg(ipv4_1).arg(ipv4_2).arg(ipv4_3).arg(ipv4_4).arg(ipv4_port);
    }
};

#endif // SERVER_INFO_H
