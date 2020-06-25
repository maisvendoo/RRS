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

    MainMenu(QString name = "Main menu", BoxType = osgWidget::Box::VERTICAL);

    void setWidth(int width);

    void addItem(QString item);

    Label *getItem(unsigned int idx);

    void init(QString cfg_path, QString fonts_dir);

private:

    int width;

    int height;

    std::string font_path;

    int font_size;

    osgWidget::Color font_color;
    osgWidget::Color back_color;
    osgWidget::Color active_color;

    std::vector<Label *> items;
};

#endif // MENU_H
