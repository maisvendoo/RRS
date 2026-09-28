#ifndef     SIGNAL_LABEL_H
#define     SIGNAL_LABEL_H

#include    <QLabel>
#include    <QObject>

class Signal;

class QAction;
class QMenu;
class QMouseEvent;
class QWidget;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SignalLabel : public QLabel
{
    Q_OBJECT

public:

    SignalLabel(QWidget* parent = nullptr);

    ~SignalLabel();

    Signal* signal = nullptr;

    QMenu* menu = nullptr;

    QAction* action_build_route = nullptr;
    QAction* action_open_train = nullptr;
    QAction* action_open_shunting = nullptr;
    QAction* action_open_call = nullptr;
    QAction* action_close = nullptr;

    bool need_train = false;
    bool need_shunting = false;
    bool need_call = false;

signals:

    void popUpMenu(Signal* signal);

public slots:

    void resetMenu();

private:

    void mousePressEvent(QMouseEvent* event);
};

#endif
