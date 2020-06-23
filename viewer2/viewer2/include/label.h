#ifndef     LABEL_H
#define     LABEL_H

#include    <QString>

#include    <osgWidget/Label>

class Label : public osgWidget::Label
{
public:

    Label(const QString &label);

    bool mousePush(double, double, const osgWidget::WindowManager *)
    {
        return true;
    }

    bool mouseEnter(double, double, const osgWidget::WindowManager *)
    {
        setColor(0.6f, 0.6f, 0.6f, 1.0f);

        return true;
    }

    bool mouseLeave(double, double, const osgWidget::WindowManager *)
    {
        setColor(0.3f, 0.3f, 0.3f, 1.0f);

        return true;
    }

private:

    wchar_t str[2048];
};

#endif // LABEL_H
