//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
#ifndef     KEYBOARD_H
#define     KEYBOARD_H

#include    <QObject>
#include    <QMap>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Keyboard : public QObject
{
    Q_OBJECT

public:

    explicit Keyboard(QObject *parent = Q_NULLPTR);
    virtual ~Keyboard();

    QMap<int, bool> getKeyMap() const;

    void setKey(int key);

    void resetKey(int key);

    QByteArray serialize();

private:

    QMap<int, bool> keys;

    void addKey(int key);
};

#endif // KEYBOARD_H
