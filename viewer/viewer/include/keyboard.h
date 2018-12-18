#ifndef     KEYBOARD_H
#define     KEYBOARD_H

#include    <QObject>
#include    <QMap>
#include    <osgGA/GUIEventHandler>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class KeyboardHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    KeyboardHandler(QObject *parent = Q_NULLPTR);

    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

protected:

    QMap<int, bool>     keys;

    void addKey(int key);

    void setKey(int key);

    void resetKey(int key);

    void init();
};

#endif // KEYBOARD_H
