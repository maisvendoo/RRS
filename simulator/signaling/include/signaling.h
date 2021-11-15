#ifndef     SIGNALING_H
#define     SIGNALING_H

#include    <QObject>

#include    "signaling-export.h"

#include    "block-section.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SIGNALING_EXPORT Signaling : public QObject
{
    Q_OBJECT

public:

    Signaling(QObject *parent = Q_NULLPTR);

    ~Signaling();

    virtual void step(double t, double dt);

protected:

    /// Блок-участки на маршруте
    std::vector<BlockSection *> sections;
};

#endif // SIGNALING_H
