#include    "gui-funcs.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgWidget::Color parse_color(QString color_str)
{
    osgWidget::Color color(1.0, 1.0, 1.0, 1.0);

    color_str.remove('#');
    color_str.remove(' ');

    if (color_str.count() != 8)
        return color;

    QString red = color_str.mid(0, 2);
    QString green = color_str.mid(2, 2);
    QString blue = color_str.mid(4, 2);
    QString alpha = color_str.mid(6, 2);

    bool isOk = false;

    color.r() = static_cast<float>(red.toInt(&isOk, 16)) / 255.0f;
    color.g() = static_cast<float>(green.toInt(&isOk, 16)) / 255.0f;
    color.b() = static_cast<float>(blue.toInt(&isOk, 16)) / 255.0f;
    color.a() = static_cast<float>(alpha.toInt(&isOk, 16)) / 255.0f;

    return color;
}
