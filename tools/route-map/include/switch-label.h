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

signals:

    void popUpMenu();

private:

    void mousePressEvent(QMouseEvent *event);

};

#endif
