#include "km-21kr2.h"



//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
Km21KR2::Km21KR2(QObject* parent) : Device(parent)
  , k21(true)
  , k22(true)
  , k23(false)

  , k31(false)
  , k32(false)
  , k33(false)

  , k01(false)
  , k02(false)

  , autoSet(false)
  , autoReset(false)
  , p(false)
  , isPressedOneTime(false)
  , hod(false)
  , reverseState(0)
  , mainShaftPos(0.0)
  , fieldWeakShaft(0.0)
  , mainShaftHeight(0.0)
{
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
Km21KR2::~Km21KR2()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Km21KR2::ode_system(const state_vector_t& Y, state_vector_t& dYdt, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
    Q_UNUSED(dYdt)
}

//------------------------------------------------------------------------------
// Загрузка данных из конфигурационного файла
//------------------------------------------------------------------------------
void Km21KR2::load_config(CfgReader& cfg)
{
    Q_UNUSED(cfg)

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Km21KR2::preStep(state_vector_t& Y, double t)
{
    Q_UNUSED(t)
    Q_UNUSED(Y)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Km21KR2::stepKeysControl(double t, double dt)
{
    Q_UNUSED(t)
    Q_UNUSED(dt)


    if (!isPressedOneTime)
        reverseState += ((getKeyState(KEY_W) && reverseState != 1) -
                         (getKeyState(KEY_S) && reverseState != -1));

    isPressedOneTime = (getKeyState(KEY_W) || getKeyState(KEY_S));

    k01 = (reverseState == 1);
    k02 = (reverseState == -1);

    controlState.k01 = k01;
    controlState.k02 = k02;

    if (reverseState == 0)
        return;

    // Авт. сброс
    if (getKeyState(KEY_D))
    {
        if (isControl())
            autoReset = false;

        if (!autoReset)
        {
            if (isShift() && fieldWeakShaft != 0 && !p && hod)
            {
                fieldWeakShaft -= 2;
            }
        }
    }

    if (autoReset)
    {
        return;
    }

    // 1 вниз
    if (getKeyState(KEY_A))
    {
        if(isShift() && fieldWeakShaft != 10 && !p && hod)
        {
            fieldWeakShaft += 2;
        }
    }


    // Авт. набор
    if (getKeyState(KEY_Q))
    {
        autoSet = true;
    }
    else
    {
        autoSet = false;
    }

    // 1 вверх
    if (getKeyState(KEY_E))
    {
        autoReset = true;
        mainShaftPos = -10;
    }

    if (!autoReset)
    {
        if (autoSet)
        {
            mainShaftPos = 4;
        }
        else
        {
            mainShaftPos = (!isShift() && !isControl()) * (-5 * getKeyState(KEY_D) +
                                                            2 * getKeyState(KEY_A));
            p = (mainShaftPos != 0);
        }
    }

    k21 = (mainShaftPos == -10 || mainShaftPos == 0 || mainShaftPos == 4);
    k22 = (mainShaftPos == 0 || mainShaftPos == 2 || mainShaftPos == 4);
    k23 = (mainShaftPos == 2 || mainShaftPos == 4);

    k31 = (fieldWeakShaft == 2 || fieldWeakShaft == 8 || fieldWeakShaft == 10);
    k32 = (fieldWeakShaft == 4 || fieldWeakShaft == 8);
    k33 = (fieldWeakShaft == 6 || fieldWeakShaft == 10);

    controlState.up = (k21 && k23);
    controlState.up1 = (!k21 && k23);
    controlState.zero = (k22 && !k23);
    controlState.down1 = (!k21 && !k22);
    controlState.down = (k21 && !k22);

    controlState.k31 = k31;
    controlState.k32 = k32;
    controlState.k33 = k33;
}

void Km21KR2::stepExternalControl(double t, double dt)
{
    connectSignals(KM_K01, k01);
    connectSignals(KM_K02, k02);
    connectSignals(KM_K21, k21);
    connectSignals(KM_K22, k22);
    connectSignals(KM_K23, k23);
    connectSignals(KM_K31, k31);
    connectSignals(KM_K32, k32);
    connectSignals(KM_K33, k33);

    if (control_signals.analogSignal[KM_K01].is_active)
        k01 = static_cast<bool>(control_signals.analogSignal[KM_K01].value);

    if (control_signals.analogSignal[KM_K02].is_active)
        k02 = static_cast<bool>(control_signals.analogSignal[KM_K02].value);

    if (control_signals.analogSignal[KM_K21].is_active)
        k21 = static_cast<bool>(control_signals.analogSignal[KM_K21].value);

    if (control_signals.analogSignal[KM_K22].is_active)
        k22 = static_cast<bool>(control_signals.analogSignal[KM_K22].value);

    if (control_signals.analogSignal[KM_K23].is_active)
        k23 = static_cast<bool>(control_signals.analogSignal[KM_K23].value);

    if (control_signals.analogSignal[KM_K31].is_active)
        k31 = static_cast<bool>(control_signals.analogSignal[KM_K31].value);

    if (control_signals.analogSignal[KM_K32].is_active)
        k32 = static_cast<bool>(control_signals.analogSignal[KM_K32].value);

    if (control_signals.analogSignal[KM_K33].is_active)
        k33 = static_cast<bool>(control_signals.analogSignal[KM_K33].value);

    controlState.k01 = k01;
    controlState.k02 = k02;

    controlState.up = (k21 && k23);
    controlState.up1 = (!k21 && k23);
    controlState.zero = (k22 && !k23);
    controlState.down1 = (!k21 && !k22);
    controlState.down = (k21 && !k22);

    controlState.k31 = k31;
    controlState.k32 = k32;
    controlState.k33 = k33;
}

void Km21KR2::connectSignals(ControllerSignals cs, bool k)
{
    if (control_signals.analogSignal[cs].is_active)
        k = static_cast<bool>(control_signals.analogSignal[cs].value);
}
