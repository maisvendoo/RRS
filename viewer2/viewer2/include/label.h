#ifndef     LABEL_H
#define     LABEL_H

#include    <QObject>

#include    <osgWidget/Label>


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Label : public QObject, public osgWidget::Label
{
    Q_OBJECT

public:

    Label(int width = 100, int height = 50);

    bool mousePush(double cx, double cy, const osgWidget::WindowManager *wm) override;

    bool mouseEnter(double cx, double cy, const osgWidget::WindowManager *wm) override;

    bool mouseLeave(double cx, double cy, const osgWidget::WindowManager *wm) override;

    bool isActive() const { return is_active; }

    osgWidget::Color    font_color;

    osgWidget::Color    back_color;

    osgWidget::Color    active_color;

private:

    std::string font_name;

    int width;

    int height;


    bool                is_active;

signals:

    void action();
};

#endif // LABEL_H
