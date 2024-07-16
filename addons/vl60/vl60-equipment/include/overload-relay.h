#ifndef     OVERLOADRELAY_H
#define     OVERLOADRELAY_H

#include    <QString>
#include    "trigger.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OverloadRelay : public Trigger
{
public:

    OverloadRelay(QObject *parent = Q_NULLPTR);

    ~OverloadRelay();

    void setCurrent(double value);

    void read_config(const QString &filename, const QString &dir_path = "");

private:

    /// Измеряемый ток
    double current;

    /// Уставка срабатывания реле
    double trig_current;
};

#endif // OVERLOADRELAY_H
