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

#if __unix__

    QMap<int, int>::iterator it = keymap.find(key);

    if (it != keymap.end())
    {
        code = keymap[key];
    }

#else

    code = key;

#endif

    return code;
}
