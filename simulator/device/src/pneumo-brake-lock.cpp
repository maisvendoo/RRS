#include    "pneumo-brake-lock.h"

#include    "math-funcs.h"
#include    "key-symbols.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoBrakeLock::PneumoBrakeLock(QObject *parent) : PneumoCombineCrane(parent)
{
    setKeySymbolChangeLockState(KEY_BackSpace);
    setKeyModifierChangeLockState(MODIFIER_OnlyAlt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoBrakeLock::~PneumoBrakeLock()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::init(double pBP, double pFL)
{
    PneumoCombineCrane::init(pBP, pFL);

    is_handle_locked = (pBP > p_lock);

    if (!is_handle_allowed)
    {
        ref_lock_state = false;
    }

    setY(LOCK_POS, static_cast<double>(ref_lock_state));
    setY(PRESSURE_FL, pFL);
    setY(PRESSURE_BC, 0.0);
    this->pFL = pFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::setKeySymbolChangeLockState(std::uint16_t key_symbol)
{
    key_symbol_change_state = key_symbol;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::setKeyModifierChangeLockState(std::uint16_t key_modifier)
{
    key_modifier_change_state = key_modifier;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::allowLockHandle(bool allow)
{
    is_handle_allowed = allow;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoBrakeLock::isLockHandleAllowed() const
{
    return is_handle_allowed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::insertLockHandle(bool insert)
{
    insert = insert && is_handle_allowed;

    if (insert)
    {
        is_lock_handle.set();
    }
    else
    {
        if (!isStateOn())
        {
            is_lock_handle.reset();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoBrakeLock::isLockHandle() const
{
    return is_lock_handle.getState();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::setStateOn(bool state)
{
    if (state)
    {
        insertLockHandle(true);

        if (isLockHandle())
        {
            ref_lock_state = true;
        }
    }
    else
    {
        ref_lock_state = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoBrakeLock::isStateOn() const
{
    return (getY(LOCK_POS) > 0.5);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoBrakeLock::isStateLockedByBPpressure() const
{
    return is_handle_locked;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoBrakeLock::getLockHandlePosition() const
{
    return getY(LOCK_POS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::setFLpressure(double value)
{
    pFL = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoBrakeLock::getCraneFLpressure() const
{
    return getY(PRESSURE_FL);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::setCraneFLflow(double value)
{
    QFLcrane = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoBrakeLock::getFLflow() const
{
    return QFL;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::setBCpressure(double value)
{
    pBC = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoBrakeLock::getCraneBCpressure() const
{
    return getY(PRESSURE_BC);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::setCraneBCflow(double value)
{
    QBCcrane = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoBrakeLock::getBCflow() const
{
    return QBC;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t PneumoBrakeLock::getSoundState(size_t idx) const
{
    if (idx < 2)
    {
        return PneumoCombineCrane::getSoundState(idx);
    }
    if (idx >= 5)
    {
        return lock_state.getSoundState(idx - 5);
    }
    return is_lock_handle.getSoundState(idx - 2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float PneumoBrakeLock::getSoundSignal(size_t idx) const
{
    if (idx < 2)
    {
        return PneumoCombineCrane::getSoundSignal(idx);
    }
    if (idx >= 5)
    {
        return lock_state.getSoundSignal(idx - 5);
    }
    return is_lock_handle.getSoundSignal(idx - 2);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::preStep(state_vector_t &Y, double t)
{
    (void) t;

    // Проверяем стопор рукоятки давлением в тормозной магистрали
    is_handle_locked = (pBP > p_lock);

    // Убираем рукоятку, когда блокировка выключена
    if (!ref_lock_state && (Y[LOCK_POS] < 0.01))
    {
        is_lock_handle.reset();
    }

    if (Y[PneumoCombineCrane::HANDLE_POS] > 0.5)
    {
        // Экстренное торможение комбинированным краном - выпуск воздуха из ТМ
        emergency_flow_sound.state = 1;
        emergency_flow_sound.volume = K_sound * cbrt(abs(QBP));

        QBP = -K_emergency * pBP;

        if (isStateOn())
        {
            // Блокировка включена - оборудование подключено к магистралям
            QFL = QFLcrane;
            QBC = QBCcrane;
            Y[PRESSURE_FL] = pFL;
            Y[PRESSURE_BC] = pBC;
        }
        else
        {
            // Блокировка выключена - оборудование отключено от магистралей
            QFL = 0.0;
            QBC = 0.0;
        }

        return;
    }

    emergency_flow_sound.state = 0;
    emergency_flow_sound.volume = 0.0f;

    if (isStateOn())
    {
        if (Y[PneumoCombineCrane::HANDLE_POS] > -0.5)
        {
            // Поездное положение комбинированного крана - пропуск воздуха в ТМ
            QBP = QBPcrane;
            Y[PneumoCombineCrane::PRESSURE_BP] = pBP;
        }
        else
        {
            // Комбинированный кран в положении двойной тяги отключает поток воздуха
            QBP = 0.0;
        }

        // Блокировка включена - оборудование подключено к магистралям
        QFL = QFLcrane;
        QBC = QBCcrane;
        Y[PRESSURE_FL] = pFL;
        Y[PRESSURE_BC] = pBC;
    }
    else
    {
        // Блокировка выключена - оборудование отключено от магистралей
        QBP = 0.0;
        QFL = 0.0;
        QBC = 0.0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    (void) t;

    // Поворот рукоятки комбинированного крана
    double ref_pos = static_cast<double>(ref_state.getPosition()) - 1.0;
    double delta = ref_pos - Y[PneumoCombineCrane::HANDLE_POS];
    if (abs(delta) > 0.05)
    {
        dYdt[PneumoCombineCrane::HANDLE_POS] = sign(delta) / switch_time;
    }
    else
    {
        dYdt[PneumoCombineCrane::HANDLE_POS] = 20.0 * delta / switch_time;
    }

    // Поворот рукоятки блокировки
    if (is_handle_locked && (ref_lock_state != isStateOn()))
    {
        // Если нужно переключение рукоятки, но оно заблокировано давлением в ТМ,
        // только немного дёргаем рукоятку
        if (ref_lock_state)
        {
            ref_pos = 0.2; // Дёргаем - поворот 0.0-0.2 вместо 0.0-1.0
            delta = ref_pos - Y[LOCK_POS];
            if (delta < Physics::ZERO)
            {
                // Когда дёрнули - сбрасываем необходимость переключения
                ref_lock_state = false;
            }
        }
        else
        {
            ref_pos = 0.8; // Дёргаем - поворот 1.0-0.8 вместо 1.0-0.0
            delta = ref_pos - Y[LOCK_POS];
            if (delta > -Physics::ZERO)
            {
                // Когда дёрнули - сбрасываем необходимость переключения
                ref_lock_state = true;
            }
        }
    }
    else
    {
        if (ref_lock_state)
        {
            ref_pos = 1.0;
            // Озвучка
            lock_state.set();
        }
        else
        {
            ref_pos = 0.0;
            // Озвучка
            lock_state.reset();
        }
        delta = ref_pos - Y[LOCK_POS];
    }

    if (abs(delta) > 0.05)
    {
        dYdt[LOCK_POS] = sign(delta) / switch_time;
    }
    else
    {
        dYdt[LOCK_POS] = 20.0 * delta / switch_time;
    }

    // Потоки в условный объём трубопроводов за блокировкой
    if (Y[LOCK_POS] > 0.5)
    {
        if (abs(Y[PneumoCombineCrane::HANDLE_POS]) < 0.5)
        {
            // Поездное положение комбинированного крана - оборудование соединено с ТМ,
            // моделировать условный объём в трубопроводах за блокировкой не нужно
            dYdt[PneumoCombineCrane::PRESSURE_BP] = 0.0;
        }
        else
        {
            // Прочие положения комбинированного крана - оборудование отключено от ТМ
            // и взаимодействует только с условным объёмом в трубопроводах за блокировкой
            dYdt[PneumoCombineCrane::PRESSURE_BP] = QBPcrane / V0;
        }

        // Блокировка включена - оборудование подключено к магистралям,
        // моделировать условный объём в трубопроводах за блокировкой не нужно
        dYdt[PRESSURE_FL] = 0.0;
        dYdt[PRESSURE_BC] = 0.0;
    }
    else
    {
        // Блокировка выключена - оборудование отключено от магистралей
        // и взаимодействует только с условным объёмом в трубопроводах за блокировкой
        dYdt[PneumoCombineCrane::PRESSURE_BP] = QBPcrane / V0;
        dYdt[PRESSURE_FL] = QFLcrane / V0;
        dYdt[PRESSURE_BC] = QBCcrane / V0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    int pos = -1;
    cfg.getInt(secName, "CombineCranePos", pos);
    setCombineCranePosition(pos);

    bool state = false;
    cfg.getBool(secName, "BrakeLockState", state);
    setStateOn(state);

    double tmp = 0.0;
    cfg.getDouble(secName, "SwitchTime", tmp);
    if (tmp > 0.1)
        switch_time = tmp;

    tmp = 0.0;
    cfg.getDouble(secName, "V0", tmp);
    if (tmp > 1e-3)
        V0 = tmp;

    cfg.getDouble(secName, "K_emergency", K_emergency);

    cfg.getDouble(secName, "K_sound", K_sound);

    cfg.getDouble(secName, "p_lock", p_lock);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoBrakeLock::stepKeysControl(double t, double dt)
{
    PneumoCombineCrane::stepKeysControl(t, dt);

    if ((::getKeyState(pressed_keys, key_symbol_change_state) && isModifier(pressed_keys, key_modifier_change_state)))
    {
        // Переключаем новым нажатием на клавишу
        if (!prev_key)
        {
            setStateOn(!ref_lock_state);
        }
        prev_key = true; // Запоминаем, что клавиша нажата
    }
    else
    {
        prev_key = false; // Запоминаем, что клавиша отпущена
    }
}
