#include "switcher.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switcher::Switcher(QObject* parent, int key_code, int kol_states) : Device(parent)
{
    setNumPositions(kol_states);
    setKeyCode(key_code);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Switcher::~Switcher()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::setKeyCode(int key_code)
{
    keyCode = key_code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::setNumPositions(int value)
{
    num_states = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::setState(int value)
{
    state = cut(value, 0, num_states - 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Switcher::getNumPositions() const
{
    return num_states;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Switcher::getPosition() const
{
    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Switcher::getHandlePosition() const
{
    return static_cast<float>(state) / static_cast<float>(num_states - 1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Switcher::isSwitched(int pos) const
{
    return pos == state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t Switcher::getSoundState(size_t idx) const
{
    (void) idx;
    return switch_sound;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float Switcher::getSoundSignal(size_t idx) const
{
    (void) idx;
    return switch_sound.createSoundSignal();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::step(double t, double dt)
{
    stepKeysControl(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::ode_system(const state_vector_t &Y,
                          state_vector_t &dYdt,
                          double t)
{
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
    Q_UNUSED(t)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Switcher::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)

    bool allow_spring_first = is_spring_first;
    bool allow_spring_last = is_spring_last;

    if (getKeyState(keyCode))
    {
        if (ableToPress)
        {
            if(isShift())
            {
                if (state < (num_states - 1))
                {
                    // Переключение вперёд
                    state++;
                    switch_sound.play();
                }
                else
                {
                    // Запрет автовозврата назад
                    allow_spring_last = false;
                }
            }
            else
            {
                if (state > 0)
                {
                    // Переключение назад
                    state--;
                    switch_sound.play();
                }
                else
                {
                    // Запрет автовозврата вперёд
                    allow_spring_first = false;
                }
            }

            // Запрещаем переключение до отпускания клавиши
            ableToPress = false;
        }
    }
    else
    {
        // Разрешаем переключение следующим нажатием клавиши
        ableToPress = true;
    }

    // Автовозврат вперёд
    if (allow_spring_first && (num_states > 1) && (state == 0))
    {
        state++;
        switch_sound.play();
    }
    // Автовозврат назад
    if (allow_spring_last && (num_states > 1) && (state == (num_states - 1)))
    {
        state--;
        switch_sound.play();
    }
}
