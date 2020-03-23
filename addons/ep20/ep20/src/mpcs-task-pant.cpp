#include    <fstream>
#include    <iostream>
#include    "qobject.h"

#include    "mpcs-task-pant.h"
#include    "current-kind.h"

#include    "ep20-signals.h"

//------------------------------------------------------------------------------
// Конструктор
//------------------------------------------------------------------------------
TaskPant::TaskPant(QObject *parent) : QObject(parent)
  , selectedCab(1)
  , last_current_kind(1)
  , taskPantStateUp(INITIAL_STATE_IS_UP)
  , taskPantStateDown(INITIAL_STATE_IS_DOWN)
  , isMainSwitch(false)
  , isHighSpeedSwitch(false)
  , ref_current_kind(last_current_kind)
  , pantPriority(0)
  , prevPant(-1)
  , downedPant(0)
{
    std::fill(ctrl_signals.begin(), ctrl_signals.end(), false);

    pantUpWaitingTimer = new Timer(10.0, false, Q_NULLPTR);
    pantDownWaitingTimer = new Timer(10.0, false, Q_NULLPTR);
    protectionDeviceWaitingTimer = new Timer(1.0, false, Q_NULLPTR);
    connect(pantUpWaitingTimer, &Timer::process, this, &TaskPant::pantUpTimerHandler);
    connect(pantDownWaitingTimer, &Timer::process, this, &TaskPant::pantDownTimerHandler);
    connect(protectionDeviceWaitingTimer, &Timer::process, this, &TaskPant::protectionDeviceTimerHandler);
}

//------------------------------------------------------------------------------
// Деструктор
//------------------------------------------------------------------------------
TaskPant::~TaskPant()
{

}

//------------------------------------------------------------------------------
// Установка контрольных сигналов
//------------------------------------------------------------------------------
void TaskPant::setControlSignal(size_t id, bool value)
{
    ctrl_signals[id] = value;
}

void TaskPant::setStoragePath(QString path)
{
    pathStorage = path;
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
void TaskPant::writeLastCurrentKind(mpcs_input_t mpcs_input)
{
    std::ofstream file(pathStorage.toStdString() + "last_current_kind");

    if (file.is_open())
    {
        file << mpcs_input.current_kind;
        file.close();
    }
    else
    {
        last_current_kind = CURRENT_AC;
    }
}

//------------------------------------------------------------------------------
// Команда на подъем ТП
//------------------------------------------------------------------------------
bool TaskPant::isCommandUp()
{
    if (ctrl_signals[FORWARD_UP])
        pantPriority = 1;

    if (ctrl_signals[BACKWARD_UP])
        pantPriority = 0;

    return ctrl_signals[FORWARD_UP] || ctrl_signals[BACKWARD_UP];
}

//------------------------------------------------------------------------------
// Команда на опускание ТП
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// Деление на цело (для проверки на опускание предыдущего поднятого ТП)
//------------------------------------------------------------------------------
bool isEven(int num)
{
    return ( (num % 2) == 0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void setPantLampsState(mpcs_output_t &mpcs_output, unsigned long pantPriority, float state)
{
    mpcs_output.lamps_state.pant_fwd.is_blinked = mpcs_output.lamps_state.pant_bwd.is_blinked = false;

    pantPriority == 1 ? mpcs_output.lamps_state.pant_fwd.state = state :
                        mpcs_output.lamps_state.pant_bwd.state = state;
}

//------------------------------------------------------------------------------
// Обработка поднятие ТП
//------------------------------------------------------------------------------
void TaskPant::pantUp(const mpcs_input_t &mpcs_input, mpcs_output_t &mpcs_output)
{
    bool is_command_up = isCommandUp();

    switch (taskPantStateUp)
    {
        case  INITIAL_STATE_IS_UP:
        {
            if (is_command_up)
            {
                taskPantStateUp = UP_PRIORETY_PANT;

                pantPriority == 1 ? mpcs_output.lamps_state.pant_fwd.is_blinked = true :
                                    mpcs_output.lamps_state.pant_bwd.is_blinked = true;
            }

            break;
        }
        case UP_PRIORETY_PANT:
        {
            mpcs_output.pant_state[pants[ref_current_kind][pantPriority]] = true;

            ctrl_signals[FORWARD_UP] = false;
            ctrl_signals[BACKWARD_UP] = false;

            taskPantStateUp = WAITING_UP;

            break;
        }
        case  WAITING_UP:
        {
            pantUpWaitingTimer->start();

            if (mpcs_input.pant_up[pants[ref_current_kind][pantPriority]])
            {
                pantUpWaitingTimer->stop();

                prevPant = static_cast<int>(pants[ref_current_kind][pantPriority]);

                taskPantStateUp = CONTROL_CURRENT_KIND;
            }
            break;
        }
        case  CONTROL_CURRENT_KIND:
        {
            if (mpcs_input.current_kind == ref_current_kind)
            {
                taskPantStateUp = LOWER_FIRST_PANT;
            }
            else
            {
                taskPantStateUp = CHANGE_KURRENT_KIND;
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

            taskPantStateUp = UP_PRIORETY_PANT;

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
                taskPantStateUp = READY;
                break;
            }

            if (more_one_up)
            {
                mpcs_output.pant_state[p] = false;
                taskPantStateUp = WAITING_DOWN_UPON_UP;
                pantDownWaitingTimer->start();
            }
            else
            {
                taskPantStateUp = READY;
            }
            break;
        }

        case  WAITING_DOWN_UPON_UP:
        {
            if (mpcs_input.pant_down[static_cast<size_t>(prevPant)])
            {
                pantDownWaitingTimer->stop();
                taskPantStateUp = READY;
            }

            break;
        }
        case  READY:
        {
            writeLastCurrentKind(mpcs_input);
            taskPantStateUp = INITIAL_STATE_IS_UP;

            setPantLampsState(mpcs_output, pantPriority, SIG_LIGHT_GREEN);

            break;
        }
        case  FAULT_IS_UP:
        {
            taskPantStateUp = READY;
            break;
        }
        default:
            break;
    }
}

//------------------------------------------------------------------------------
// Обработка опускания ТП
//------------------------------------------------------------------------------
void TaskPant::pantDown(const mpcs_input_t &mpcs_input, mpcs_output_t &mpcs_output)
{
    bool is_command_down = isCommandDown();

    switch (taskPantStateDown)
    {
        case INITIAL_STATE_IS_DOWN:
        {
           if (is_command_down)
           {
               taskPantStateDown = IS_OFF_MS_HSS;

               pantPriority == 1 ? mpcs_output.lamps_state.pant_fwd.is_blinked = true :
                                   mpcs_output.lamps_state.pant_bwd.is_blinked = true;
           }

           break;
        }
        case IS_OFF_MS_HSS:
        {
            if (!isMainSwitch && !isHighSpeedSwitch)
            {
                taskPantStateDown = LOWER_PANT;
            }
            else
            {
                taskPantStateDown = TURN_OFF_PROTECTION_DEVICE;
            }

            break;
        }
        case TURN_OFF_PROTECTION_DEVICE:
        {

            isMainSwitch = true;
            isHighSpeedSwitch = true;
            taskPantStateDown = WAITING_TURN_OFF;

            break;
        }
        case WAITING_TURN_OFF:
        {
            protectionDeviceWaitingTimer->start();

            if(isMainSwitch && isHighSpeedSwitch)
            {
                protectionDeviceWaitingTimer->stop();

                taskPantStateDown = LOWER_PANT;
            }
            else
            {
                taskPantStateDown = FAULT_IS_DOWN;
            }

            break;
        }
        case LOWER_PANT:
        {
            int downedPant = pants[ref_current_kind][pantPriority];
            mpcs_output.pant_state[downedPant] = false;
            ctrl_signals[FORWARD_DOWN] = false;
            ctrl_signals[BACKWARD_DOWN] = false;

            taskPantStateDown = WAITING_DOWN_UPON_DOWN;
            pantDownWaitingTimer->start();

            break;
        }
        case WAITING_DOWN_UPON_DOWN:
        {
            if (mpcs_input.pant_down[static_cast<size_t>(downedPant)])
            {
                pantDownWaitingTimer->stop();
                taskPantStateDown = INITIAL_STATE_IS_DOWN;
                setPantLampsState(mpcs_output, pantPriority, SIG_LIGHT_YELLOW);
            }

            break;
        }
        case FAULT_IS_DOWN:
        {
            taskPantStateDown = INITIAL_STATE_IS_DOWN;
            break;
        }
    }
}

//------------------------------------------------------------------------------
// Шаг моделирования
//------------------------------------------------------------------------------
void TaskPant::step(state_vector_t &Y, double t, double dt,
                    const mpcs_input_t &mpcs_input, mpcs_output_t &mpcs_output)
{
    pantUp(mpcs_input, mpcs_output);

    pantDown(mpcs_input, mpcs_output);

    pantUpWaitingTimer->step(t, dt);
    pantDownWaitingTimer->step(t, dt);
    protectionDeviceWaitingTimer->step(t, dt);
}

//------------------------------------------------------------------------------
// Таймер поднятия ТП
//------------------------------------------------------------------------------
void TaskPant::pantUpTimerHandler()
{
    taskPantStateUp = FAULT_IS_UP;
}

//------------------------------------------------------------------------------
// Таймер опускания ТП
//------------------------------------------------------------------------------
void TaskPant::pantDownTimerHandler()
{
    taskPantStateUp = FAULT_IS_UP;
}

//------------------------------------------------------------------------------
// Таймер отключения АЗ
//------------------------------------------------------------------------------
void TaskPant::protectionDeviceTimerHandler()
{
    taskPantStateDown = FAULT_IS_DOWN;
}
