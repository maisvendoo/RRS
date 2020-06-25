#include    "esc-press-handler.h"

#include    <osgViewer/Viewer>
#include    <osgWidget/WindowManager>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EscPressHandler::EscPressHandler(osgWidget::Window *menu,
                                 osgWidget::WindowManager *wm)
    : QObject(Q_NULLPTR)
    , osgGA::GUIEventHandler()
    , menu(menu)
    , wm(wm)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EscPressHandler::handle(const osgGA::GUIEventAdapter &ea,
                    osgGA::GUIActionAdapter &aa)
{
    if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
    {
        if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Escape)
        {
            osgViewer::Viewer *v = static_cast<osgViewer::Viewer *>(&aa);
            showMenu(v);
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EscPressHandler::slotHide()
{
    menu->hide();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EscPressHandler::showMenu(osgViewer::Viewer *v)
{
    const osg::GraphicsContext::Traits *traits = v->getCamera()->getGraphicsContext()->getTraits();

    int width = traits->width;
    int height = traits->height;

    wm->setWidth(width);
    wm->setHeight(height);
    wm->resizeAllWindows();

    menu->show();
}
