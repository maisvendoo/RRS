#include    "menu.h"

#include    "CfgReader.h"
#include    "gui-funcs.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MainMenu::MainMenu(QString name, osgWidget::Box::BoxType box_type)
    : osgWidget::Box(name.toStdString(), box_type, true)
    , width(200)
    , height(36)
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
    osg::ref_ptr<Label> label = new Label(width, height);
    label->setFont(font_path);
    label->setFontSize(static_cast<unsigned int>(font_size));

    label->font_color = font_color;
    label->back_color = back_color;
    label->active_color = active_color;

    size_t text_size = static_cast<size_t>(item.count()) + 1;
    wchar_t *text = new wchar_t[text_size];
    memset(text, 0, sizeof(wchar_t) * text_size);
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MainMenu::init(QString cfg_path, QString fonts_dir)
{
    CfgReader cfg;

    if (!cfg.load(cfg_path))
        return;

    QString secName = "MainMenu";

    cfg.getInt(secName, "Width", width);
    cfg.getInt(secName, "ItemHeight", height);

    QString font_name;
    cfg.getString(secName, "Font", font_name);
    font_path = (fonts_dir + QDir::separator() + font_name + ".ttf").toStdString();

    cfg.getInt(secName, "FontSize", font_size);

    QString tmp;
    cfg.getString(secName, "FontColor", tmp);
    font_color = parse_color(tmp);

    cfg.getString(secName, "BackColor", tmp);
    back_color = parse_color(tmp);

    cfg.getString(secName, "ActiveColor", tmp);
    active_color = parse_color(tmp);
}


