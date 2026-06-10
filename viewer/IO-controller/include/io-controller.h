#ifndef     IO_CONTROLLER_H
#define     IO_CONTROLLER_H

#include    <io-controller-export.h>
#include    <QObject>
#include    <set>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IO_CONTROLLER_EXPORT IOController : public QObject
{
    Q_OBJECT

public:

    IOController(QObject *parent = nullptr);

    ~IOController() = default;

    void setPressedKey(uint16_t keyBase);

    void setReleasedKey(uint16_t keyBase);

    virtual void step(float t, float dt);

protected:

    /// Массив нажатых клавиш
    std::set<uint16_t> _pressed_keys;

    enum ControlType
    {
        CTRL_TYPE_KEYBOARD,
        CTRL_TYPE_MOUSE,
        CTRL_TYPE_CTRL_PANEL
    };

    /// Обработка клавиатурного управления
    virtual void processKeyBoardInput();

    /// Обработка управления мышью
    virtual void processMouseInput();

    /// Обработка управления с пульта тренажера
    virtual void processControlPanelInput();

private:

    /// Обработка управления
    void processControl(const ControlType &ctrl_type);
};

#endif
