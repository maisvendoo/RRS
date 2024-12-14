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

#ifndef     KEYBOARD_H
#define     KEYBOARD_H

#include    <QObject>
//#include    <QMap>
#include    <QSet>
#include    <QSharedMemory>
#include    <osgGA/GUIEventHandler>

/*!
 * \class
 * \brief Keyboard event handler and state keeper
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class KeyboardHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    /// Constructor
    KeyboardHandler(QObject *parent = Q_NULLPTR);
    ~KeyboardHandler();

    /// Keyboard handler
    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

    std::vector<int> getPressedKeys();

signals:

    /// Send keyboard state to server
    void sendKeyBoardState();

protected:
/*
    /// Map to store keys state (keymap)
    QMap<int, bool>     keys;
*/
    /// Set to store pressed keys (keymap)
    QSet<int>     keys;

    /// Set to store available keys
    QSet<int>     available_keys;

    /// Add key to available keys
    void addKey(int key);

    /// Set key in keymap as pressed
    void setKey(int key);

    /// Get key state
    bool getKey(int key);

    /// Reset key in keymap (set as not pressed)
    void resetKey(int key);

    bool setKeyState(int key, bool state);

    /// Keymap initialization
    void init();
/*
    /// Serialization of keymap to transmite thow TCP-socket
    QByteArray serialize();

    QSharedMemory   keys_data;

    void sendKeysData(const QByteArray &data);
*/
    bool modkeyProcess(const osgGA::GUIEventAdapter &ea);
};

#endif // KEYBOARD_H
