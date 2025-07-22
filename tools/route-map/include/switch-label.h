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

    SwitchLabel(QWidget *parent = nullptr);

    ~SwitchLabel();

    Connector *conn;

    QMenu *menu = nullptr;

    QAction *action_switch_fwd = nullptr;

    QAction *action_switch_bwd = nullptr;

signals:

    void popUpMenu();

public slots:

    void resetMenu();

private:

    void mousePressEvent(QMouseEvent *event);
};

#endif
