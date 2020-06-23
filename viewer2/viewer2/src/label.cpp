#include    "label.h"

Label::Label(int width, int height)
  : QObject(nullptr)
  , osgWidget::Label()
  , font_name("../fonts/arial.ttf")
  , width(width)
  , height(height)
  , font_color(osg::Vec4(1.0, 1.0, 1.0, 1.0))
  , back_color(osg::Vec4(0.25, 0.25, 0.25, 1.0))
  , active_color(osg::Vec4(0.0, 0.5, 0.0, 1.0))
  , is_active(false)
{
    setFont(font_name);
    setFontColor(font_color);
    setColor(back_color);
    setFontSize(18);
    addWidth(width);
    addHeight(height);
    setEventMask(osgWidget::EVENT_MOUSE_PUSH | osgWidget::EVENT_MASK_MOUSE_MOVE);
    setAlignHorizontal(osgWidget::Label::HA_CENTER);
}

bool Label::mousePush(double cx, double cy, const osgWidget::WindowManager *wm)
{
    emit action();
    return true;
}

bool Label::mouseEnter(double cx, double cy, const osgWidget::WindowManager *wm)
{
    setColor(active_color);

    is_active = true;

    return true;
}

bool Label::mouseLeave(double cx, double cy, const osgWidget::WindowManager *wm)
{
    setColor(back_color);

    is_active = false;

    return true;
}
