//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

#ifndef     TCP_CONFIG_H
#define     TCP_CONFIG_H

#include    <QtGlobal>
#include    <QString>

/*!
 * \struct
 * \brief Server's configuration
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct tcp_sever_config_t
{
    /// Server name
    QString     name;
    /// Listener TCP-port
    quint16     port;

    tcp_sever_config_t()
        : name("simulator")
        , port(1992)
    {

    }
};

/*!
 * \struct
 * \brief Clinet's configuration
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct tcp_client_config_t
{
    /// Client name
    QString         name;
    /// Host address
    QString         address;
    /// Host port
    quint16         port;
    /// Time interval between try of connection
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
