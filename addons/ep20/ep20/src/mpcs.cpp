#include <fstream>
#include <iostream>

#include "mpcs.h"
#include "current-kind.h"


//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
MPCS::MPCS(QObject *parent) : Device(parent)
  , selectedCab(1)
  , last_current_kind(1)
  , taskPantState(INITIAL_STATE)
  , ref_current_kind(last_current_kind)
  , pantPriority(0)
  , prevPant(-1)
{
    pantUpWaitingTimer = new Timer(10.0, false);
    connect(pantUpWaitingTimer, &Timer::process, this, &MPCS::pantUpTimerHandler);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
MPCS::~MPCS()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::setSignalInputMPCS(const mpcs_input_t &mpcs_input)
{
    this->mpcs_input = mpcs_input;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::setStoragePath(QString path)
{
    pathStorage = path;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::readLastCurrentKind()
{
    ifstream file(pathStorage.toStdString() + "last_current_kind");

    if (file.is_open())
    {
        file >> last_current_kind;
        file.close();
        ref_current_kind = last_current_kind;
    }
    else
    {
        last_current_kind = CURRENT_AC;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::writeLastCurrentKind()
{
    ofstream file(pathStorage.toStdString() + "last_current_kind");

    if (file.is_open())
    {
        file << last_current_kind;
        file.close();
    }
    else
    {
        last_current_kind = CURRENT_AC;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::taskPantUp(state_vector_t &Y, double t)
{
    switch (taskPantState)
    {
        case  INITIAL_STATE:
        {
            if (pantControlButton.getState())
            {
                taskPantState = UP_PRIORETY_PANT;
            }
            break;
        }
        case UP_PRIORETY_PANT:
        {
            mpcs_output.pant_state[pants[ref_current_kind][pantPriority]] = true;
            taskPantState = WAITING_UP;
            break;
        }
        case  WAITING_UP:
        {
            pantUpWaitingTimer->start();

            if (mpcs_input.pant_up[pants[ref_current_kind][pantPriority]])
            {
                pantUpWaitingTimer->stop();

                prevPant = static_cast<int>(pants[ref_current_kind][pantPriority]);

                taskPantState = CONTROL_CURRENT_KIND;
            }
            break;
        }
        case  CONTROL_CURRENT_KIND:

            if (mpcs_input.current_kind == ref_current_kind)
            {
                taskPantState = LOWER_FIRST_PANT;
            }
            else
            {
                taskPantState = CHANGE_KURRENT_KIND;
            }
            break;

        case  CHANGE_KURRENT_KIND:
        {
            if (ref_current_kind == CURRENT_AC)
            {
                ref_current_kind = CURRENT_DC;
            }
            else
            {
                ref_current_kind = CURRENT_AC;
            }

            taskPantState = UP_PRIORETY_PANT;

            break;
        }
        case  LOWER_FIRST_PANT:
        {
            if (prevPant < 0)
            {
                taskPantState = WAITING_DOWN;
            }
            else
            {
                mpcs_output.pant_state[static_cast<size_t>(prevPant)] = false;
            }

            break;
        }
        case  UP_RESERVE_PANT:
        {
            pantPriority++;

            if (pantPriority > 1)
            {
                taskPantState = FAULT;
            }
            else
            {
                mpcs_output.pant_state[pants[ref_current_kind][pantPriority]] = true;
                taskPantState = WAITING_UP;
            }

            break;
        }
        case  WAITING_DOWN:
        {

            break;
        }
        case  READY:
        {

            break;
        }
        case  FAULT:
        {


            break;
        }
        default:
            break;
    }
}

void MPCS::stepKeysControl(double t, double dt)
{
    if (getKeyState(KEY_I))
    {
        if (pantControlButton.getState())
        {
            pantControlButton.reset();
        }
        else
        {
            pantControlButton.set();
        }
    }

    pantUpWaitingTimer->step(t, dt);
}

void MPCS::pantUpTimerHandler()
{
    taskPantState = UP_RESERVE_PANT;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
mpcs_output_t MPCS::getSignalOutputMPCS()
{
    return mpcs_output;
}

void MPCS::init()
{
    pant_group_t dc_group;
    pant_group_t ac_group;

    switch (selectedCab)
    {
    case 1:

        dc_group[0] = PANT_DC2;
        dc_group[1] = PANT_DC1;
        ac_group[0] = PANT_AC2;
        ac_group[1] = PANT_AC1;
        break;

    case 2:
        dc_group[1] = PANT_DC2;
        dc_group[0] = PANT_DC1;
        ac_group[1] = PANT_AC2;
        ac_group[0] = PANT_AC1;

        break;
    }

    pants.insert(CURRENT_AC, ac_group);
    pants.insert(CURRENT_DC, dc_group);

    readLastCurrentKind();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void MPCS::preStep(state_vector_t &Y, double t)
{
    taskPantUp(Y, t);

    // Подъем приоритетного токоприеника данного рода тока
//     mpcs_output.pant_state[pants[last_current_kind][0]] = true;


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
