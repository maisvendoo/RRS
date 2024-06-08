#ifndef     EXTARNAL_SIGNAL_H
#define     EXTARNAL_SIGNAL_H

#include    <QMetaType>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct signal_t
{
    float       cur_value;
    float       prev_value;
    bool    is_active;

    signal_t()
        : cur_value(0)
        , prev_value(0)
        , is_active(false)
    {

    }

    void setValue(float value)
    {
        prev_value = cur_value;
        cur_value = value;
    }

    bool isChanged() const
    {
        float eps = 0.01f;
        return abs(prev_value - cur_value) < eps;
    }   
};

Q_DECLARE_METATYPE(signal_t)

#endif // EXTARNAL_SIGNAL_H
