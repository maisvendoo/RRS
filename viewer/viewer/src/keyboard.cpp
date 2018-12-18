#include    "keyboard.h"

#include    <QDataStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeyboardHandler::KeyboardHandler(QObject *parent)
    : QObject(parent)
    , osgGA::GUIEventHandler ()
{
    init();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool KeyboardHandler::handle(const osgGA::GUIEventAdapter &ea,
                             osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:

        setKey(ea.getKey());
        emit sendKeyBoardState(serialize());
        break;

    case osgGA::GUIEventAdapter::KEYUP:

        resetKey(ea.getKey());
        emit sendKeyBoardState(serialize());
        break;

    default:


        break;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyboardHandler::addKey(int key)
{
    keys.insert(key, false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyboardHandler::setKey(int key)
{
    QMap<int, bool>::iterator it = keys.find(key);

    if (it != keys.end())
        keys[key] = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyboardHandler::resetKey(int key)
{//------------------------------------------------------------------------------
    //
    //------------------------------------------------------------------------------
    QMap<int, bool>::iterator it = keys.find(key);

    if (it != keys.end())
        keys[key] = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyboardHandler::init()
{
    addKey(osgGA::GUIEventAdapter::KEY_A);
    addKey(osgGA::GUIEventAdapter::KEY_B);
    addKey(osgGA::GUIEventAdapter::KEY_C);
    addKey(osgGA::GUIEventAdapter::KEY_D);
    addKey(osgGA::GUIEventAdapter::KEY_E);
    addKey(osgGA::GUIEventAdapter::KEY_F);
    addKey(osgGA::GUIEventAdapter::KEY_H);
    addKey(osgGA::GUIEventAdapter::KEY_I);
    addKey(osgGA::GUIEventAdapter::KEY_J);
    addKey(osgGA::GUIEventAdapter::KEY_K);
    addKey(osgGA::GUIEventAdapter::KEY_L);
    addKey(osgGA::GUIEventAdapter::KEY_M);
    addKey(osgGA::GUIEventAdapter::KEY_N);
    addKey(osgGA::GUIEventAdapter::KEY_O);
    addKey(osgGA::GUIEventAdapter::KEY_P);
    addKey(osgGA::GUIEventAdapter::KEY_Q);
    addKey(osgGA::GUIEventAdapter::KEY_R);
    addKey(osgGA::GUIEventAdapter::KEY_S);
    addKey(osgGA::GUIEventAdapter::KEY_T);
    addKey(osgGA::GUIEventAdapter::KEY_V);
    addKey(osgGA::GUIEventAdapter::KEY_W);
    addKey(osgGA::GUIEventAdapter::KEY_X);
    addKey(osgGA::GUIEventAdapter::KEY_Y);
    addKey(osgGA::GUIEventAdapter::KEY_Z);

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray KeyboardHandler::serialize()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << keys;

    return data;
}
