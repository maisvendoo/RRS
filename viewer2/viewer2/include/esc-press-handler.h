#ifndef     ESC_PRESS_HANDLER_H
#define     ESC_PRESS_HANDLER_H

#include    <QObject>

#include    <osgGA/GUIEventHandler>
#include    <osgWidget/Window>

class EscPressHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    EscPressHandler(osgWidget::Window *menu)
        : QObject(Q_NULLPTR)
        , osgGA::GUIEventHandler()
        , menu(menu)
    {

    }

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &)
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Escape)
            {
                menu->show();
            }
        }

        return false;
    }

public slots:

    void slotHide()
    {
        menu->hide();
    }

private:

    osgWidget::Window *menu;
};

#endif // ESC_PRESS_HANDLER_H
