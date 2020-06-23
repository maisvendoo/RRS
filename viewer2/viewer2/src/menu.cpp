#include    "menu.h"

MainMenu::MainMenu(QString name, osgWidget::Box::BoxType box_type)
    : osgWidget::Box (name.toStdString(), box_type, false)
{

}
