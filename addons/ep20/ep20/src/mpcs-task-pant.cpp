#include <fstream>
#include <iostream>
#include "qobject.h"

#include "mpcs-task-pant.h"
#include "current-kind.h"

TaskPant::TaskPant(QObject *parent) : QObject(parent)
  , selectedCab(1)
  , last_current_kind(1)
  , taskPantState(INITIAL_STATE)
  , ref_current_kind(last_current_kind)
  , pantPriority(0)
  , prevPant(-1)
{
    std::fill(ctrl_signals.begin(), ctrl_signals.end(), false);

    pantUpWaitingTimer = new Timer(10.0, false, Q_NULLPTR);
    pantDownWaitingTimer = new Timer(10.0, false, Q_NULLPTR);
    connect(pantUpWaitingTimer, &Timer::process, this, &TaskPant::pantUpTimerHandler);
    connect(pantDownWaitingTimer, &Timer::process, this, &TaskPant::pantDownTimerHandler);
}

TaskPant::~TaskPant()
{

}

void TaskPant::setControlSignal(size_t id, bool value)
{
    ctrl_signals[id] = value;
}

//------------------------------------------------------------------------------
// Чтение из файла значения последнего активного рода тока
//------------------------------------------------------------------------------
void TaskPant::readLastCurrentKind()
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
void TaskPant::writeLastCurrentKind()
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

bool TaskPant::isCommandUp()
{
    if (ctrl_signals[FORWARD_UP])
        pantPriority = 1;

    if (ctrl_signals[BACKWARD_UP])
        pantPriority = 0;

    return ctrl_signals[FORWARD_UP] || ctrl_signals[BACKWARD_UP];
}

bool TaskPant::isCommandDown()
{
    if (ctrl_signals[FORWARD_DOWN])
        pantPriority = 1;

    if (ctrl_signals[BACKWARD_DOWN])
        pantPriority = 0;

    return ctrl_signals[FORWARD_DOWN] || ctrl_signals[BACKWARD_DOWN];
}

//------------------------------------------------------------------------------
// Инициализация (Определение группы ТП в соответсвии с номером кабины)
//------------------------------------------------------------------------------
void TaskPant::init()
{
    switch (selectedCab)
    {
    case 1:

        pants[CURRENT_DC].push_back(PANT_DC2);
        pants[CURRENT_DC].push_back(PANT_DC1);
        pants[CURRENT_AC].push_back(PANT_AC2);
        pants[CURRENT_AC].push_back(PANT_AC1);

        break;

    case 2:

        pants[CURRENT_DC].push_back(PANT_DC1);
        pants[CURRENT_DC].push_back(PANT_DC2);
        pants[CURRENT_AC].push_back(PANT_AC1);
        pants[CURRENT_AC].push_back(PANT_AC2);

        break;
    }

    readLastCurrentKind();
}

bool isEven(int num)
{
    return ( (num % 2) == 0);
}

void TaskPant::pantUp(const mpcs_input_t &mpcs_input, mpcs_output_t &mpcs_output)
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
            mpcs_output.pant_state[pants[ref_current_kind][pantPriority]] = true;

            ctrl_signals[FORWARD_UP] = false;
            ctrl_signals[BACKWARD_UP] = false;

            taskPantState = WAITING_UP;

            break;
        }

        case DOWN_PRIORETY_PANT:
        {
            mpcs_output.pant_state[pants[ref_current_kind][pantPriority]] = false;

            ctrl_signals[FORWARD_DOWN] = false;
            ctrl_signals[BACKWARD_DOWN] = false;

            taskPantState = WAITING_DOWN;

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

            if (!(isEven(p) ^ isEven(prevPant)))
            {
                taskPantState = READY;
                break;
            }

            if (more_one_up)
            {
                mpcs_output.pant_state[p] = false;
                taskPantState = WAITING_DOWN;
                pantDownWaitingTimer->start();
            }
            else
            {
                taskPantState = READY;
            }
            break;
        }

        case  WAITING_DOWN:
        {
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
}

void TaskPant::step(state_vector_t &Y, double t, double dt, const mpcs_input_t &mpcs_input, mpcs_output_t &mpcs_output)
{
    pantUp(mpcs_input, mpcs_output);

    pantUpWaitingTimer->step(t, dt);
    pantDownWaitingTimer->step(t, dt);
}

void TaskPant::pantUpTimerHandler()
{
    taskPantState = FAULT;
}

void TaskPant::pantDownTimerHandler()
{
    taskPantState = FAULT;
}
