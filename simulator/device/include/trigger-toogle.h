#ifndef     TRIGGER_TOOGLE_H
#define     TRIGGER_TOOGLE_H

#include    "trigger.h"
#include    "key-symbols.h"

class DEVICE_EXPORT TriggerToogle : public Trigger
{
public:

    TriggerToogle(std::uint16_t key_code = KEY_Undefined);

    ~TriggerToogle() = default;

    /// Задать управляющую клавишу
    void setKeyCode(std::uint16_t key_code);

    /// Управляющие сигналы
    void setControl(std::set<uint16_t>* keys = nullptr);

    /// Шаг симуляции для управления триггером
    virtual void step(double t = 0.0, double dt = 0.0);

protected:

    /// Управляющая клавиша
    std::uint16_t keyCode = KEY_Undefined;

    bool is_prev_keyCode = false;

    std::set<uint16_t>* pressed_keys = nullptr;

    bool getKeyState(std::uint16_t key) const;
};

#endif // TRIGGER_TOOGLE_H
