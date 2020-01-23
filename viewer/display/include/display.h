#ifndef     DISPLAY_H
#define     DISPLAY_H

#include    <QWidget>
#include    "display-export.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DISPLAY_EXPORT AbstractDisplay : public QWidget
{
public:

    AbstractDisplay(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    virtual ~AbstractDisplay();


protected:



};

#endif // DISPLAY_H
