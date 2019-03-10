//------------------------------------------------------------------------------
//
//      Keyboard processing to transmite keyboard state to server
//      (c) maisvendoo, 18/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */

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

    keys_data.setKey("keys");
    keys_data.attach();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool KeyboardHandler::handle(const osgGA::GUIEventAdapter &ea,
                             osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(aa)

    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:
        {

            int key = ea.getKey();

            if (!getKey(key))
            {
                setKey(ea.getKey());
                //emit sendKeyBoardState(serialize());
                sendKeysData(serialize());
            }

            break;
        }

    case osgGA::GUIEventAdapter::KEYUP:
        {

            int key = ea.getKey();

            if (getKey(key))
            {
                resetKey(ea.getKey());
                //emit sendKeyBoardState(serialize());
                sendKeysData(serialize());
            }

            break;
        }

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
bool KeyboardHandler::getKey(int key)
{
    QMap<int, bool>::iterator it = keys.find(key);

    if (it != keys.end())
        return keys[key];

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyboardHandler::resetKey(int key)
{
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
    addKey(osgGA::GUIEventAdapter::KEY_G);
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

    addKey(osgGA::GUIEventAdapter::KEY_Leftbracket);
    addKey(osgGA::GUIEventAdapter::KEY_Rightbracket);
    addKey(osgGA::GUIEventAdapter::KEY_Quote);
    addKey(osgGA::GUIEventAdapter::KEY_Semicolon);

    addKey(osgGA::GUIEventAdapter::KEY_Shift_L);
    addKey(osgGA::GUIEventAdapter::KEY_Shift_R);
    addKey(osgGA::GUIEventAdapter::KEY_Control_L);
    addKey(osgGA::GUIEventAdapter::KEY_Control_R);
    addKey(osgGA::GUIEventAdapter::KEY_Alt_L);
    addKey(osgGA::GUIEventAdapter::KEY_Alt_R);

    addKey(osgGA::GUIEventAdapter::KEY_1);
    addKey(osgGA::GUIEventAdapter::KEY_2);
    addKey(osgGA::GUIEventAdapter::KEY_3);
    addKey(osgGA::GUIEventAdapter::KEY_4);
    addKey(osgGA::GUIEventAdapter::KEY_5);
    addKey(osgGA::GUIEventAdapter::KEY_6);
    addKey(osgGA::GUIEventAdapter::KEY_7);
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyboardHandler::sendKeysData(const QByteArray &data)
{
    if (keys_data.lock())
    {
        memcpy(keys_data.data(), data.data(), static_cast<size_t>(data.size()));

        keys_data.unlock();
    }
}
