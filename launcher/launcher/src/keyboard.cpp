//------------------------------------------------------------------------------
//
//      Keyboard processor
//      (c) maisvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Keyboard processor
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#include    "keyboard.h"

#include    <QDataStream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Keyboard::Keyboard(QObject *parent) : QObject(parent)
{
    addKey(Qt::Key_Escape);
    addKey(Qt::Key_Shift);
    addKey(Qt::Key_Control);
    addKey(Qt::Key_Alt);
    addKey(Qt::Key_Tab);
    addKey(Qt::Key_CapsLock);
    addKey(Qt::Key_Enter);
    addKey(Qt::Key_Backspace);
    addKey(Qt::Key_Space);

    addKey(Qt::Key_Left);
    addKey(Qt::Key_Right);
    addKey(Qt::Key_Up);
    addKey(Qt::Key_Down);

    addKey(Qt::Key_F1);
    addKey(Qt::Key_F2);
    addKey(Qt::Key_F3);
    addKey(Qt::Key_F4);
    addKey(Qt::Key_F5);
    addKey(Qt::Key_F6);
    addKey(Qt::Key_F7);
    addKey(Qt::Key_F8);
    addKey(Qt::Key_F9);
    addKey(Qt::Key_F10);
    addKey(Qt::Key_F11);
    addKey(Qt::Key_F12);

    addKey(Qt::Key_0);
    addKey(Qt::Key_1);
    addKey(Qt::Key_2);
    addKey(Qt::Key_3);
    addKey(Qt::Key_4);
    addKey(Qt::Key_5);
    addKey(Qt::Key_6);
    addKey(Qt::Key_7);
    addKey(Qt::Key_8);
    addKey(Qt::Key_9);

    addKey(Qt::Key_A);
    addKey(Qt::Key_B);
    addKey(Qt::Key_C);
    addKey(Qt::Key_D);
    addKey(Qt::Key_E);
    addKey(Qt::Key_F);
    addKey(Qt::Key_G);
    addKey(Qt::Key_I);
    addKey(Qt::Key_J);
    addKey(Qt::Key_K);
    addKey(Qt::Key_L);
    addKey(Qt::Key_M);
    addKey(Qt::Key_N);
    addKey(Qt::Key_O);
    addKey(Qt::Key_P);
    addKey(Qt::Key_Q);
    addKey(Qt::Key_R);
    addKey(Qt::Key_S);
    addKey(Qt::Key_T);
    addKey(Qt::Key_V);
    addKey(Qt::Key_W);
    addKey(Qt::Key_X);
    addKey(Qt::Key_Y);
    addKey(Qt::Key_Z);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Keyboard::~Keyboard()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QMap<int, bool> Keyboard::getKeyMap() const
{
    return keys;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Keyboard::setKey(int key)
{
    QMap<int, bool>::iterator it = keys.find(key);

    if (it != keys.end())
        keys[key] = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Keyboard::resetKey(int key)
{
    QMap<int, bool>::iterator it = keys.find(key);

    if (it != keys.end())
        keys[key] = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QByteArray Keyboard::serialize()
{
    QByteArray data;
    QDataStream *stream = new QDataStream(&data, QIODevice::WriteOnly);

    (*stream) << keys;
    delete stream;

    return data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Keyboard::addKey(int key)
{
    keys.insert(key, false);
}


