#ifndef     RS485_H
#define     RS485_H

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct port_config_t
{
    QString     name;
    int         baudrate;
    int         data_bits;
    int         stop_bits;
    int         parity;
    int         timeout;

    port_config_t()
        : name("/dev/ttyUSB0")
        , baudrate(115200)
        , data_bits(8)
        , stop_bits(1)
        , parity(0)
        , timeout(100)
    {

    }
};

#endif // RS485_H
