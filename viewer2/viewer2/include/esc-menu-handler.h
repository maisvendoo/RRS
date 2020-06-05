#ifndef     ESC_MENU_HANDLER_H
#define     ESC_MENU_HANDLER_H

#include    <QObject>
#include    <QMenu>

#include    <osgGA/GUIEventHandler>
#include    <osgViewer/View>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EscMenuHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    EscMenuHandler(QObject *parent = Q_NULLPTR);

    bool handle(const osgGA::GUIEventAdapter &ea,
                osgGA::GUIActionAdapter &aa) override;

private:

    QMenu   *menu;    

    void showMenu(osgViewer::View *view);
};


#endif // ESC_MENU_HANDLER_H
