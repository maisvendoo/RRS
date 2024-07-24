#ifndef SWITCHER_H
#define SWITCHER_H

#include    "device.h"

//------------------------------------------------------------------------------
// Многопозиционный переключатель
//------------------------------------------------------------------------------
class DEVICE_EXPORT Switcher : public Device
{
public:

    Switcher(QObject *parent = Q_NULLPTR, int key_code = 0, int kol_states = 0);

    ~Switcher();

    /// Задать управляющую клавишу
    void setKeyCode(int key_code);

    /// Задать количество позиций
    void setNumPositions(int value);

    /// Задать подпружиненный автовозврат из первой позиции в следующую
    void setSpringFirst(bool is_spring = true);

    /// Задать подпружиненный автовозврат из последней позиции в предыдущую
    void setSpringLast(bool is_spring = true);

    /// Задать позицию
    void setState(int value);

    /// Количество позиций
    int getNumPositions() const;

    /// Текущая позиция
    int getPosition() const;

    /// Текущее относительное положение переключателя, 0.0 - 1.0
    float getHandlePosition() const;

    /// Состояние позиции
    bool isSwitched(int pos) const;

    /// Звук переключения позиции
    virtual sound_state_t getSoundState(size_t idx = 0) const;

    /// Сигнал звука переключения позиции
    virtual float getSoundSignal(size_t idx = 0) const;

    virtual void step(double t, double dt);

protected:

    /// Управляющая клавиша
    int keyCode = 0;

    /// Текущая позиция
    int state = 0;

    /// Количество позиций переключателя
    int num_states = 1;

    /// Признак подпружиненного автовозврата из первой позиции в следующую
    bool is_spring_first = false;

    /// Признак подпружиненного автовозврата из последней позиции в предыдущую
    bool is_spring_last = false;

    /// Признак возможности однократного переключения по следующему нажатию клавиши
    bool ableToPress = false;

    /// Звук переключения позиции
    sound_state_t switch_sound = sound_state_t();

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t);

    void stepKeysControl(double t, double dt);
};

#endif // SWITCHER_H
