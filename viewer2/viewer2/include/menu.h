#ifndef     MENU_H
#define     MENU_H

#include    <QString>

#include    <osgWidget/Box>

#include    "label.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MainMenu : public osgWidget::Box
{
public:

    MainMenu(QString name = "Main menu", BoxType box_type = osgWidget::Box::VERTICAL);

    void setWidth(int width);

    void addItem(QString item);

    Label *getItem(unsigned int idx);

private:

    int width;

    int height;

    std::vector<Label *> items;
};

#endif // MENU_H
