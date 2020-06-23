#include    "label.h"

#include    <osgText/String>

Label::Label(const QString &label) : osgWidget::Label()
{
    label.toWCharArray(str);

    setFont("../fonts/dejavu-sans-mono.ttf");
    setLabel(osgText::String(str));
    setFontSize(14);
    setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
    setColor(1.0f, 0.0f, 0.0f, 1.0);
    addHeight(18);
    setCanFill(true);
    setEventMask(osgWidget::EVENT_MOUSE_PUSH | osgWidget::EVENT_MASK_MOUSE_MOVE);
}
