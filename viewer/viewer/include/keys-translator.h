//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     KEYS_TRANSLATOR_H
#define     KEYS_TRANSLATOR_H

#include    <QObject>
#include    <QMap>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class KeysTranslator : public QObject
{
    Q_OBJECT

public:

    KeysTranslator(QObject *parent = Q_NULLPTR);

    virtual ~KeysTranslator();

    int translate(int key);

protected:

    QMap<int, int>  keymap;
};

#endif // KEYS_TRANSLATOR_H
