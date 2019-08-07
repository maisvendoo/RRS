#ifndef MPCSTASK_H
#define MPCSTASK_H

#include    "mpcs-data.h"
#include    "solver-types.h"
#include    "device-export.h"

#include    <QObject>

class DEVICE_EXPORT Task : public QObject
{
    Q_OBJECT

public:

    Task(QObject *parent = Q_NULLPTR);

    virtual ~Task();

    virtual void init();

    virtual void step(state_vector_t &Y, double t,
                      double dt,
                      const mpcs_input_t &mpcs_input,
                      mpcs_output_t &mpcs_output);
};


#endif // MPCSTASK_H
