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

    SignalLabel(QWidget *parent = nullptr);

    ~SignalLabel();

    Signal *signal;

    QMenu *menu = nullptr;

    QAction *action_open = nullptr;

    QAction *action_close = nullptr;

signals:

    void popUpMenu();

public slots:

    void resetMenu();

private:

    void mousePressEvent(QMouseEvent *event);
};

#endif
