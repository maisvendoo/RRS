#ifndef SWITCHER_CONTROL_H
#define SWITCHER_CONTROL_H

#include    "switcher.h"
#include    "key-symbols.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT SwitcherControl : public Switcher
{
public:

    SwitcherControl(std::uint16_t key_code = KEY_Undefined, std::uint16_t num_positions = 3);

    ~SwitcherControl() = default;

    /// Задать управляющую клавишу
    void setKeyCode(std::uint16_t key_code);

    /// Управляющие сигналы
    void setControl(std::set<uint16_t>* keys = nullptr);

    /// Задать подпружиненный автовозврат из первой позиции в следующую
    void setSpringFirst(bool is_spring = true);

    /// Задать подпружиненный автовозврат из последней позиции в предыдущую
    void setSpringLast(bool is_spring = true);

    /// Шаг симуляции для управления триггером
    virtual void step(double t = 0.0, double dt = 0.0);

protected:

    /// Управляющая клавиша
    std::uint16_t keyCode = 0;

    /// Признак подпружиненного автовозврата из первой позиции в следующую
    bool is_spring_first = false;

    /// Признак подпружиненного автовозврата из последней позиции в предыдущую
    bool is_spring_last = false;

    /// Признак возможности однократного переключения по следующему нажатию клавиши
    bool no_prev_keyCode = false;

    std::set<uint16_t>* pressed_keys = nullptr;

    bool getKeyState(std::uint16_t key) const;
};

#endif // SWITCHER_CONTROL_H
