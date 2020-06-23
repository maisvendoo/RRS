#ifndef     MENU_H
#define     MENU_H

#include    <QString>

#include    <osgWidget/Box>

#include    "label.h"

class MainMenu : public osgWidget::Box
{
public:

    MainMenu(QString name = "Main menu", BoxType box_type = osgWidget::Box::VERTICAL);



private:
};

#endif // MENU_H
