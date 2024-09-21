#ifndef     SIGNAL_LABEL_H
#define     SIGNAL_LABEL_H

#include    <QLabel>
#include    <rail-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SignalLabel : public QLabel
{
    Q_OBJECT

public:

    SignalLabel(QWidget *parent = Q_NULLPTR);

    ~SignalLabel();

    Signal *signal;

signals:

    void popUpMenu();

private:

    void mousePressEvent(QMouseEvent *event);
};

#endif
