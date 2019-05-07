#include    "keys-translator.h"

#include    <osgGA/GUIEventAdapter>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeysTranslator::KeysTranslator(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeysTranslator::~KeysTranslator()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int KeysTranslator::translate(int key)
{
    if (key <= 0xFF)
        return key;

    int code = 0;

#if defined(Q_OS_WIN)

    code = key;

#else

    QMap<int, int>::iterator it = keymap.find(key);

    if (it != keymap.end())
    {
        code = keymap[key];
    }

#endif

    return code;
}
