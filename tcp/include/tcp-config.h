//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     TCP_CONFIG_H
#define     TCP_CONFIG_H

#include    <QtGlobal>
#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct tcp_sever_config_t
{
    QString     name;
    quint16     port;

    tcp_sever_config_t()
        : name("simulator")
        , port(1992)
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct tcp_client_config_t
{
    QString         name;
    QString         address;
    quint16         port;
    unsigned int    reconect_interval;

    tcp_client_config_t()
        : name("")
        , address("127.0.0.1")
        , port(1992)
        , reconect_interval(1000)
    {

    }
};

#endif // TCP_CONFIG_H
