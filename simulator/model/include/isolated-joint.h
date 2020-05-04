#ifndef     ISOLATED_JOINT_H
#define     ISOLATED_JOINT_H

#include    "connector.h"
#include    "signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IsolatedJoint : public Connector
{
public:

    IsolatedJoint(QObject *parent = Q_NULLPTR);

    ~IsolatedJoint();

    void configure(QDomNode secNode);

private:

    /// Сигнал, установленный у данного изостыка
    Signal  *signal;
};

#endif // ISOLATED_JOINT_H
