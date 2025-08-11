#ifndef SWITCHER_CONTROL_H
#define SWITCHER_CONTROL_H

#include    "switcher.h"
#include    "key-symbols.h"

//------------------------------------------------------------------------------
// Многопозиционный переключатель с управлением по сигналам от клавиатуры
// Коды управляющих клавиш и модификаторы см. в файле key-symbols.h
// Также с возможностью задать автовозврат из крайних положений
//------------------------------------------------------------------------------
class DEVICE_EXPORT SwitcherControl : public Switcher
{
public:

    SwitcherControl(std::uint16_t num_positions = 3);

    ~SwitcherControl() = default;

    /// Задать управляющую клавишу для переключения в следующую позицию
    void setKeySymbolIncrease(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор для переключения в следующую позицию
    void setKeyModifierIncrease(std::uint16_t key_modifier);

    /// Задать управляющую клавишу для переключения в предыдущую позицию
    void setKeySymbolDecrease(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор для переключения в предыдущую позицию
    void setKeyModifierDecrease(std::uint16_t key_modifier);

    /// Управляющие сигналы
    void setControl(std::set<uint16_t>* keys = nullptr);

    /// Задать подпружиненный автовозврат из первой позиции в следующую
    void setSpringFirst(bool is_spring = true);

    /// Задать подпружиненный автовозврат из последней позиции в предыдущую
    void setSpringLast(bool is_spring = true);

    /// Шаг симуляции для управления триггером
    virtual void step(double t = 0.0, double dt = 0.0);

protected:

    /// Управляющая клавиша переключения в следующую позицию
    std::uint16_t key_symbol_inc = KEY_Undefined;

    /// Клавиша-модификатор переключения в следующую позицию
    std::uint16_t key_modifier_inc = KEY_Undefined;

    /// Управляющая клавиша переключения в предыдущую позицию
    std::uint16_t key_symbol_dec = KEY_Undefined;

    /// Клавиша-модификатор переключения в предыдущую позицию
    std::uint16_t key_modifier_dec = KEY_Undefined;

    /// Признак подпружиненного автовозврата из первой позиции в следующую
    bool is_spring_first = false;

    /// Признак подпружиненного автовозврата из последней позиции в предыдущую
    bool is_spring_last = false;

    /// Предыдущее состояние клавиши переключения в следующую позицию
    bool prev_key_inc = false;

    /// Предыдущее состояние клавиши переключения в предыдущую позицию
    bool prev_key_dec = false;

    std::set<uint16_t>* pressed_keys = nullptr;
};

#endif // SWITCHER_CONTROL_H
