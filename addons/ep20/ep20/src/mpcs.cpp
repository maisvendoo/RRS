#include <fstream>
#include <iostream>

#include "mpcs.h"
#include "current-kind.h"


//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
MPCS::MPCS(QObject *parent) : Device(parent)
{

}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
MPCS::~MPCS()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::init()
{
    taskPant = new TaskPant();

    taskPant->init();
}

void MPCS::setSignalInputMPCS(const mpcs_input_t &mpcs_input)
{
    this->mpcs_input = mpcs_input;
}

void MPCS::setStoragePath(QString path)
{
    pathStorage = path;
}

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
}

void MPCS::stepDiscrete(double t, double dt)
{   
    taskPant->step(y, t, dt, mpcs_input, mpcs_output);
}

//------------------------------------------------------------------------------
// Пердварительные шаги
//------------------------------------------------------------------------------
void MPCS::preStep(state_vector_t &Y, double t)
{

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

}
