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

    /// Constructor
    explicit Keyboard(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~Keyboard();

    /// Get current keys state
    QMap<int, bool> getKeyMap() const;

    /// Set key by code
    void setKey(int key);

    /// Reset key by code
    void resetKey(int key);

    /// Serialization of keys QMap container
    QByteArray serialize();

private:

    /// Map for store keys state
    QMap<int, bool> keys;

    /// Add key to map
    void addKey(int key);
};

#endif // KEYBOARD_H
