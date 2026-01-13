#ifndef     TRAIN_LABEL_H
#define     TRAIN_LABEL_H

#include    <QLabel>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainLabel : public QLabel
{
    Q_OBJECT

public:

    TrainLabel(QWidget *parent = nullptr);

    ~TrainLabel();

    QMenu *menu = nullptr;

    QAction *rename_train = nullptr;

signals:

    void popUpMenu();

public slots:

    void resetMenu();

private:

    void mousePressEvent(QMouseEvent *event);
};

#endif
