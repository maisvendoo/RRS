#ifndef     SWITCH_LABEL_H
#define     SWITCH_LABEL_H

#include    <QLabel>
#include    <connector.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SwitchLabel : public QLabel
{
    Q_OBJECT

public:

    SwitchLabel(QWidget *parent = Q_NULLPTR);

    ~SwitchLabel();

    Connector *conn;

    QMenu *menu = Q_NULLPTR;

    QAction *action_switch_fwd = Q_NULLPTR;

    QAction *action_switch_bwd = Q_NULLPTR;

signals:

    void popUpMenu();

public slots:

    void resetMenu();

private:

    void mousePressEvent(QMouseEvent *event);
};

#endif
