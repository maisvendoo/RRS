#include    "menu.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainMenu::MainMenu(QString name, osgWidget::Box::BoxType box_type)
    : osgWidget::Box (name.toStdString(), box_type, false)
    , width(200)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainMenu::setWidth(int width)
{
    this->width = width;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainMenu::addItem(QString item)
{
    int h = 36;

    osg::ref_ptr<Label> label = new Label(width, h);
    label->setFont("../fonts/arial.ttf");

    size_t size = static_cast<size_t>(item.count()) + 1;
    wchar_t *text = new wchar_t[size];
    memset(text, 0, sizeof(wchar_t) * size);
    item.toWCharArray(text);
    label->setLabel(text);

    this->addWidget(label.get());

    items.push_back(label.get());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Label *MainMenu::getItem(unsigned int idx)
{
    return items[idx];
}


