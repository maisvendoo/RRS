#include <fstream>
#include <iostream>

#include "mpcs.h"
#include "current-kind.h"


//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
MPCS::MPCS(QObject *parent) : Device(parent)
{
    p_prev = 0;

    p_min = 0.75;

    p_max = 0.9;

    std:fill(mk_start.begin(), mk_start.end(), 0);

    mkStartTimer.setTimeout(1.0);
    connect(&mkStartTimer, &Timer::process, this, &MPCS::slotMKStart);

    mk_count = 0;
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
MPCS::~MPCS()
{

}

//------------------------------------------------------------------------------
// Инициализация
//------------------------------------------------------------------------------
void MPCS::init()
{
    taskPant = new TaskPant();

    taskPant->init();

    auxConv = new AuxiliaryConverter();  
}

//------------------------------------------------------------------------------
// Установить входной сигнал МПСУ
//------------------------------------------------------------------------------
void MPCS::setSignalInputMPCS(const mpcs_input_t &mpcs_input)
{
    this->mpcs_input = mpcs_input;
}

//------------------------------------------------------------------------------
// Установить путь к файлу
//------------------------------------------------------------------------------
void MPCS::setStoragePath(QString path)
{
    pathStorage = path;
}

//------------------------------------------------------------------------------
// Получить выходной сигнал МПСУ
//------------------------------------------------------------------------------
mpcs_output_t MPCS::getSignalOutputMPCS()
{
    return mpcs_output;
}

//------------------------------------------------------------------------------
//  Управление клавишами
//------------------------------------------------------------------------------
void MPCS::stepKeysControl(double t, double dt)
{
    if (getKeyState(KEY_I))
    {
        if (isShift())
        {
            taskPant->setControlSignal(BACKWARD_UP, true);
        }
        else
        {
            taskPant->setControlSignal(BACKWARD_DOWN, true);
        }
    }

    if (getKeyState(KEY_U))
    {
        if (isShift())
        {
            taskPant->setControlSignal(FORWARD_UP, true);
        }
        else
        {
            taskPant->setControlSignal(FORWARD_DOWN, true);
        }
    }

    if (getKeyState(KEY_P))
    {
        if (isShift())
            ms_fs_on.set();
        else
            ms_fs_on.reset();
    }

}

//------------------------------------------------------------------------------
// Шаг
//------------------------------------------------------------------------------
void MPCS::stepDiscrete(double t, double dt)
{   
    taskPant->step(y, t, dt, mpcs_input, mpcs_output);

    stepMainSwitchControl(t, dt);

    stepFastSwitchControl(t, dt);

    stepToggleSwitchMK(t, dt);
}

//------------------------------------------------------------------------------
// Шаг контроля ГВ
//------------------------------------------------------------------------------
void MPCS::stepMainSwitchControl(double t, double dt)
{
    mpcs_output.turn_on_ms = ms_fs_on.getState();
    mpcs_output.turn_on_ms = mpcs_output.turn_on_ms && mpcs_input.isOff_fs;
    mpcs_output.turn_on_ms = mpcs_output.turn_on_ms && (mpcs_input.current_kind_switch_state == 0);
    mpcs_output.turn_on_ms = mpcs_output.turn_on_ms && (mpcs_input.Uin_ms >= 19000.0)
            && (mpcs_input.Uin_ms <= 29000.0);
}

//------------------------------------------------------------------------------
// Шаг контроля БВ
//------------------------------------------------------------------------------
void MPCS::stepFastSwitchControl(double t, double dt)
{
    mpcs_output.turn_on_fs = ms_fs_on.getState();
    mpcs_output.turn_on_fs = mpcs_output.turn_on_fs && mpcs_input.isOff_ms;
    mpcs_output.turn_on_fs = mpcs_output.turn_on_fs && (mpcs_input.current_kind_switch_state == 1);
    mpcs_output.turn_on_fs = mpcs_output.turn_on_fs && (mpcs_input.Uin_fs >= 2200.0)
            && (mpcs_input.Uin_fs <= 4000.0);
}

//------------------------------------------------------------------------------
// Шаг контроля MK
//------------------------------------------------------------------------------
void MPCS::stepToggleSwitchMK(double t, double dt)
{
    bool is_all_mk_started = true;

    for (size_t i = 0; i < mpcs_output.toggleSwitchMK.size(); ++i)
    {
        is_all_mk_started = is_all_mk_started && static_cast<bool>(mk_start[i]);
        mpcs_output.toggleSwitchMK[i] = static_cast<bool>(mk_start[i]);
    }

    if ( (mpcs_input.aux_const_U[3] >= 300.0))
    {
        if (!mkStartTimer.isStarted() && !is_all_mk_started)
        {
            mk_count = 0;
            mkStartTimer.start();
        }
    }

    mkStartTimer.step(t, dt);
}

void MPCS::PressureReg()
{
    double dp = mpcs_input.PressMR - p_prev;

    if (mpcs_input.PressMR < p_min)
        mpcs_output.MKstate = 1.0;

    if (mpcs_input.PressMR > p_max)
        mpcs_output.MKstate = 0.0;

    if ( (mpcs_input.PressMR >= p_min) && (mpcs_input.PressMR <= p_max) )
    {
        mpcs_output.MKstate = hs_p(dp);
    }
}


//------------------------------------------------------------------------------
// Пердварительные шаги
//------------------------------------------------------------------------------
void MPCS::preStep(state_vector_t &Y, double t)
{
    PressureReg();
}

void MPCS::postStep(state_vector_t &Y, double t)
{
    p_prev = mpcs_input.PressMR;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::load_config(CfgReader &cfg)
{
    Q_UNUSED(cfg)
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::slotMKStart()
{
    mk_start[mk_count] = 1;

    mk_count++;

    if (mk_count == 2)
        mkStartTimer.stop();
}
