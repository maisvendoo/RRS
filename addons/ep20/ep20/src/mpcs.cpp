#include <fstream>
#include <iostream>

#include "mpcs.h"
#include "current-kind.h"


//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
MPCS::MPCS(QObject *parent) : Device(parent)
  , selectedCab(2)
  , last_current_kind(1)
  , taskPantState(INITIAL_STATE)
  , ref_current_kind(last_current_kind)
  , pantPriority(0)
  , prevPant(-1)
{
    pantUpWaitingTimer = new Timer(10.0, false, Q_NULLPTR);
    pantDownWaitingTimer = new Timer(10.0, false, Q_NULLPTR);
    connect(pantUpWaitingTimer, &Timer::process, this, &MPCS::pantUpTimerHandler);
    connect(pantDownWaitingTimer, &Timer::process, this, &MPCS::pantDownTimerHandler);
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
MPCS::~MPCS()
{

}

//------------------------------------------------------------------------------
// установить входной сигнал
//------------------------------------------------------------------------------
void MPCS::setSignalInputMPCS(const mpcs_input_t &mpcs_input)
{
    this->mpcs_input = mpcs_input;
}

//------------------------------------------------------------------------------
// Устаноновить путь к файлу значения последнего активного рода тока
//------------------------------------------------------------------------------
void MPCS::setStoragePath(QString path)
{
    pathStorage = path;
}

//------------------------------------------------------------------------------
// Считать файл значения последнего активного рода тока
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
// Записать значение рода тока в файл
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
// Конечный автомат (Задача поднятия ТП)
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
            int p = pants[ref_current_kind][pantPriority];

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
            bool more_one_up = false;
            size_t p = 0;

            for (size_t i = 0; i < mpcs_input.pant_up.size(); ++i)
            {
                if (i != prevPant)
                {
                    more_one_up = more_one_up || mpcs_input.pant_up[i];

                    if (mpcs_input.pant_up[i])
                        p = i;
                }
            }

            if (more_one_up)
            {
                mpcs_output.pant_state[p] = false;
            }
            else
            {
                taskPantState = WAITING_DOWN;
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
            pantDownWaitingTimer->start();

            if (mpcs_input.pant_down[static_cast<size_t>(prevPant)])
            {
                pantDownWaitingTimer->stop();

                taskPantState = READY;
            }

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

//------------------------------------------------------------------------------
//  Управление клавишами
//------------------------------------------------------------------------------
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
    pantDownWaitingTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//  Управление таймером поднятия ТП
//------------------------------------------------------------------------------
void MPCS::pantUpTimerHandler()
{
    taskPantState = UP_RESERVE_PANT;
}

void MPCS::pantDownTimerHandler()
{
    taskPantState = FAULT;
}

//------------------------------------------------------------------------------
// Получить выходной сигнал МПСУ
//------------------------------------------------------------------------------
mpcs_output_t MPCS::getSignalOutputMPCS()
{
    return mpcs_output;
}

void MPCS::init()
{

    switch (selectedCab)
    {
    case 1:


        //ac_group[0] = PANT_AC2;
        //ac_group[1] = PANT_AC1;

        pants[CURRENT_DC].push_back(PANT_DC2);
        pants[CURRENT_DC].push_back(PANT_DC1);
        pants[CURRENT_AC].push_back(PANT_AC2);
        pants[CURRENT_AC].push_back(PANT_AC1);
        //pants.insert(CURRENT_AC, std::array<size_t, NUM_PANTS_GROUP>({2, 0}));

        break;

    case 2:

        //pants.insert(CURRENT_DC, std::array<size_t, NUM_PANTS_GROUP>({PANT_DC1, PANT_DC2}));
        //pants.insert(CURRENT_AC, std::array<size_t, NUM_PANTS_GROUP>({PANT_AC1, PANT_AC2}));

        pants[CURRENT_DC].push_back(PANT_DC1);
        pants[CURRENT_DC].push_back(PANT_DC2);
        pants[CURRENT_AC].push_back(PANT_AC1);
        pants[CURRENT_AC].push_back(PANT_AC2);

        break;
    }


    readLastCurrentKind();
}

//------------------------------------------------------------------------------
// Пердварительные шаги
//------------------------------------------------------------------------------
void MPCS::preStep(state_vector_t &Y, double t)
{
    taskPantUp(Y, t);
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
