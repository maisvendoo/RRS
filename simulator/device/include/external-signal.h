#ifndef     EXTARNAL_SIGNAL_H
#define     EXTARNAL_SIGNAL_H

#include    <QMetaType>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct signal_t
{
    float       value;
    bool    is_active;

    signal_t()
        : value(0)
        , is_active(false)
    {

    }
};

Q_DECLARE_METATYPE(signal_t)

#endif // EXTARNAL_SIGNAL_H
