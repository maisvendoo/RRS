#include    "esc-menu-handler.h"

#include    <QAction>
#include    <QApplication>

#include    <osg/GraphicsContext>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
EscMenuHandler::EscMenuHandler(QObject *parent)
    : QObject(parent)
    , osgGA::GUIEventHandler()
    , menu(new QMenu)
{
    QAction *returnAction = new QAction(tr("Return to game"));
    QAction *saveAction = new QAction(tr("Save game"));
    QAction *optionsAction = new QAction(tr("Options"));
    QAction *quitAction = new QAction(tr("Quit game"));

    menu->addAction(returnAction);
    menu->addAction(saveAction);
    menu->addAction(optionsAction);
    menu->addSeparator();
    menu->addAction(quitAction);    

    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool EscMenuHandler::handle(const osgGA::GUIEventAdapter &ea,
                            osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Escape)
            {
                osgViewer::View *view = static_cast<osgViewer::View *>(&aa);
                showMenu(view);
            }

            break;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void EscMenuHandler::showMenu(osgViewer::View *view)
{
    const osg::GraphicsContext::Traits *ts = view->getCamera()->getGraphicsContext()->getTraits();

    int x = ts->x;
    int y = ts->y;
    int width = ts->width;
    int height = ts->height;

    int center_x = x + width / 2;
    int center_y = y + height / 2;

    // Обязательно показываем меню, чтобы инициализировались его размеры
    menu->show();

    // и только после этого перемещаем в центр клиентской области
    int w = menu->width();
    int h = menu->height();

    menu->setGeometry(center_x - w / 2, center_y - h / 2, w, h);
}
