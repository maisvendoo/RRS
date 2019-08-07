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

void MPCS::init()
{
    taskPantUp = new TaskPantUp();
    taskPantUp->init();
    // вызвать инит класс такс пант ап
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
//        if (getKeyState(KEY_Shift_L) || getKeyState(KEY_Shift_R))
//        {
            if (pantControlButton.getState())
            {
                pantControlButton.reset();
            }
            else
            {
                pantControlButton.set();
            }
//        }
//        else
//        {

//        }
    }    
}

void MPCS::stepDiscrete(double t, double dt)
{
    taskPantUp->setControlSignal(FORWARD_UP, false);
    taskPantUp->setControlSignal(BACWARD_UP, pantControlButton.getState());
    taskPantUp->step(y, t, dt, mpcs_input, mpcs_output);
    // Вызвать степ класса такс пант ап
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
