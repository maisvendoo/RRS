#include <fstream>
#include <iostream>
#include "qobject.h"

#include "mpcs-task-pant-up.h"
#include "current-kind.h"

TaskPantUp::TaskPantUp(QObject *parent) : QObject(parent)
  , selectedCab(1)
  , last_current_kind(1)
  , taskPantState(INITIAL_STATE)
  , ref_current_kind(last_current_kind)
  , pantPriority(0)
  , prevPant(-1)
{
    pantUpWaitingTimer = new Timer(10.0, false, Q_NULLPTR);
    pantDownWaitingTimer = new Timer(10.0, false, Q_NULLPTR);
    connect(pantUpWaitingTimer, &Timer::process, this, &TaskPantUp::pantUpTimerHandler);
    connect(pantDownWaitingTimer, &Timer::process, this, &TaskPantUp::pantDownTimerHandler);
}

TaskPantUp::~TaskPantUp()
{

}

void TaskPantUp::setControlSignal(size_t id, bool value)
{
    ctrl_signals[id] = value;
}

//------------------------------------------------------------------------------
// Чтение из файла значения последнего активного рода тока
//------------------------------------------------------------------------------
void TaskPantUp::readLastCurrentKind()
{
    std::ifstream file(pathStorage.toStdString() + "last_current_kind");

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
void TaskPantUp::writeLastCurrentKind()
{
    std::ofstream file(pathStorage.toStdString() + "last_current_kind");

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

bool TaskPantUp::isCommandUp()
{
    if (ctrl_signals[FORWARD_UP])
        pantPriority = 1;

    if (ctrl_signals[BACWARD_UP])
        pantPriority = 0;

    return ctrl_signals[FORWARD_UP] || ctrl_signals[BACWARD_UP];
}

//------------------------------------------------------------------------------
// Инициализация(Задача поднятия ТП)
//------------------------------------------------------------------------------
void TaskPantUp::init()
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

void TaskPantUp::step(state_vector_t &Y, double t, double dt, const mpcs_input_t &mpcs_input, mpcs_output_t &mpcs_output)
{
    bool is_command_up = isCommandUp();

    switch (taskPantState)
    {
        case  INITIAL_STATE:
        {
            if (is_command_up)
            {
                taskPantState = UP_PRIORETY_PANT;
            }
            break;
        }
        case UP_PRIORETY_PANT:
        {
            //int p = pants[ref_current_kind][pantPriority];

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
        {
            if (mpcs_input.current_kind == ref_current_kind)
            {
                taskPantState = LOWER_FIRST_PANT;
            }
            else
            {
                taskPantState = CHANGE_KURRENT_KIND;
            }
            break;
        }
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

        /*case  UP_RESERVE_PANT:
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
        }*/

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
            taskPantState = INITIAL_STATE;
            break;
        }
        case  FAULT:
        {
            taskPantState = INITIAL_STATE;
            break;
        }
        default:

            break;
    }

    pantUpWaitingTimer->step(t, dt);
    pantDownWaitingTimer->step(t, dt);
}

void TaskPantUp::pantUpTimerHandler()
{
    taskPantState = FAULT;
}

void TaskPantUp::pantDownTimerHandler()
{
    taskPantState = FAULT;
}
