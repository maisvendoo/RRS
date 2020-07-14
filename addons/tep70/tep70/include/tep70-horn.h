#ifndef     TEP70_HORN_H
#define     TEP70_HORN_H

#include    "train-horn.h"

class TEP70Horn : public TrainHorn
{
public:

    TEP70Horn(QObject *parent = Q_NULLPTR);

    ~TEP70Horn();

private:

};

#endif // TEP70_HORN_H
