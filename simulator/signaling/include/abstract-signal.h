#ifndef     ABSTRACT_SIGNAL_H
#define     ABSTRACT_SIGNAL_H

#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    NUM_LENS = 5
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
using lens_state_t = std::array<bool, NUM_LENS>;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Signal : public QObject
{
    Q_OBJECT

public:

    Signal(QObject *parent = Q_NULLPTR);

    ~Signal();

    lens_state_t getAllLensState() const { return lens_state; };

protected:

     lens_state_t lens_state;
};

#endif // ABSTRACT_SIGNAL_H
