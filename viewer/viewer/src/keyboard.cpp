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
#include    "global-const.h"

#include    <QDataStream>

#include    "cyrilic-translator.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeyboardHandler::KeyboardHandler(QObject *parent)
    : QObject(parent)
    , osgGA::GUIEventHandler ()
{
    init();

    keys_data.setKey(SHARED_MEMORY_KEYS_DATA);
    if (keys_data.attach())
    {
        OSG_FATAL << "Connected to shared memory for keysboard processing" << std::endl;
    }
    else
    {
        OSG_FATAL << "Can't connect to shared memory for keysboard processing" << std::endl;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeyboardHandler::~KeyboardHandler()
{
    keys_data.detach();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool KeyboardHandler::handle(const osgGA::GUIEventAdapter &ea,
                             osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(aa)

    modkeyProcess(ea);

    CyrilicTranslator ct;

    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:
        {
            int key = ct.translate(ea.getUnmodifiedKey());

            if (!getKey(key))
            {
                setKey(key);
                sendKeysData(serialize());
            }

            break;
        }

    case osgGA::GUIEventAdapter::KEYUP:
        {
            int key = ct.translate(ea.getUnmodifiedKey());

            if (getKey(key))
            {
                resetKey(key);
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
void KeyboardHandler::setKeyState(int key, bool state)
{
    QMap<int, bool>::iterator it = keys.find(key);

    if (it != keys.end())
        keys[key] = state;
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
    addKey(osgGA::GUIEventAdapter::KEY_U);
    addKey(osgGA::GUIEventAdapter::KEY_W);
    addKey(osgGA::GUIEventAdapter::KEY_X);
    addKey(osgGA::GUIEventAdapter::KEY_Y);
    addKey(osgGA::GUIEventAdapter::KEY_Z);

    addKey(osgGA::GUIEventAdapter::KEY_Leftbracket);
    addKey(osgGA::GUIEventAdapter::KEY_Rightbracket);
    addKey(osgGA::GUIEventAdapter::KEY_Quote);
    addKey(osgGA::GUIEventAdapter::KEY_Semicolon);
    addKey(osgGA::GUIEventAdapter::KEY_Comma);
    addKey(osgGA::GUIEventAdapter::KEY_Period);
    addKey(osgGA::GUIEventAdapter::KEY_Slash);
    addKey(osgGA::GUIEventAdapter::KEY_Backslash);

    addKey(osgGA::GUIEventAdapter::KEY_Menu);

    addKey(osgGA::GUIEventAdapter::KEY_Shift_L);
    addKey(osgGA::GUIEventAdapter::KEY_Shift_R);
    addKey(osgGA::GUIEventAdapter::KEY_Control_L);
    addKey(osgGA::GUIEventAdapter::KEY_Control_R);
    addKey(osgGA::GUIEventAdapter::KEY_Alt_L);
    addKey(osgGA::GUIEventAdapter::KEY_Alt_R);

    addKey(osgGA::GUIEventAdapter::KEY_Backquote);
    addKey(osgGA::GUIEventAdapter::KEY_1);
    addKey(osgGA::GUIEventAdapter::KEY_2);
    addKey(osgGA::GUIEventAdapter::KEY_3);
    addKey(osgGA::GUIEventAdapter::KEY_4);
    addKey(osgGA::GUIEventAdapter::KEY_5);
    addKey(osgGA::GUIEventAdapter::KEY_6);
    addKey(osgGA::GUIEventAdapter::KEY_7);
    addKey(osgGA::GUIEventAdapter::KEY_8);
    addKey(osgGA::GUIEventAdapter::KEY_9);
    addKey(osgGA::GUIEventAdapter::KEY_0);
    addKey(osgGA::GUIEventAdapter::KEY_Minus);
    addKey(osgGA::GUIEventAdapter::KEY_Equals);
    addKey(osgGA::GUIEventAdapter::KEY_BackSpace);

    addKey(osgGA::GUIEventAdapter::KEY_Space);

    addKey(osgGA::GUIEventAdapter::KEY_Delete);
    addKey(osgGA::GUIEventAdapter::KEY_Insert);
    addKey(osgGA::GUIEventAdapter::KEY_Home);
    addKey(osgGA::GUIEventAdapter::KEY_End);
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool getBit(int mask, int bit)
{
    return static_cast<bool>( mask & (1 << bit) );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeyboardHandler::modkeyProcess(const osgGA::GUIEventAdapter &ea)
{
    int modkey_mask = ea.getModKeyMask();

    setKeyState(osgGA::GUIEventAdapter::KEY_Shift_L, getBit(modkey_mask, 0));
    setKeyState(osgGA::GUIEventAdapter::KEY_Shift_R, getBit(modkey_mask, 1));

    setKeyState(osgGA::GUIEventAdapter::KEY_Control_L, getBit(modkey_mask, 2));
    setKeyState(osgGA::GUIEventAdapter::KEY_Control_R, getBit(modkey_mask, 3));

    setKeyState(osgGA::GUIEventAdapter::KEY_Alt_L, getBit(modkey_mask, 4));
    setKeyState(osgGA::GUIEventAdapter::KEY_Alt_R, getBit(modkey_mask, 5));
}
