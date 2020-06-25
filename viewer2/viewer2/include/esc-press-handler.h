#ifndef     ESC_PRESS_HANDLER_H
#define     ESC_PRESS_HANDLER_H

#include    <QObject>

#include    <osgGA/GUIEventHandler>
#include    <osgWidget/Window>

class EscPressHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    EscPressHandler(osgWidget::Window *menu, osgWidget::WindowManager *wm);


    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

public slots:

    void slotHide();

private:

    osgWidget::Window *menu;

    osgWidget::WindowManager *wm;

    void showMenu(osgViewer::Viewer *v);
};

#endif // ESC_PRESS_HANDLER_H
