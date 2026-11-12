#include    "pneumo-hose.h"

#include    "CfgReader.h"

#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHose::PneumoHose(QObject *parent) : Device(parent)
{
    name = QString("BP");

    input_signals.resize(HOSE_SIZE_OF_INPUTS);
    std::fill(input_signals.begin(), input_signals.end(), 0.0);

    output_signals.resize(HOSE_SIZE_OF_OUTPUTS);
    output_signals = {0.0, 0.0, 1.0, 0.435, 0.157, 0.0, 0.0, 0.0};
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
PneumoHose::~PneumoHose()
{

}

//------------------------------------------------------------------------------
// Legacy
//------------------------------------------------------------------------------
void PneumoHose::setKeySymbolConnect(std::uint16_t key_symbol)
{
    key_symbol_inc = key_symbol;
}

//------------------------------------------------------------------------------
// Legacy
//------------------------------------------------------------------------------
void PneumoHose::setKeyModifierConnect(std::uint16_t key_modifier)
{
    key_modifier_inc = key_modifier;
}

//------------------------------------------------------------------------------
// Legacy
//------------------------------------------------------------------------------
void PneumoHose::setKeySymbolDisconnect(std::uint16_t key_symbol)
{
    key_symbol_dec = key_symbol;
}

//------------------------------------------------------------------------------
// Legacy
//------------------------------------------------------------------------------
void PneumoHose::setKeyModifierDisconnect(std::uint16_t key_modifier)
{
    key_modifier_dec = key_modifier;
}

//------------------------------------------------------------------------------
// Legacy
//------------------------------------------------------------------------------
void PneumoHose::setControl(std::set<uint16_t>* keys,
                            control_signals_t* control_signals)
{
    Device::setControl(keys, control_signals);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::connect()
{
    // Команда соединить рукава
    if (state == FREE)
        state = DO_CONNECT;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::disconnect()
{
    // Команда разъединить рукава
    if (state > FREE)
    {
        if (state == CONNECTED)
        {
            // Для разъединения соединительные головки нужно повернуть вверх
            const double new_head_up_angle = getY(HEAD_UP_ANGLE) + ready_to_connect_head_up_angle;
            setY(HEAD_UP_ANGLE, new_head_up_angle);

            // И дополнительный импульс на ещё поворот вверх
            setY(DOWN_ANGLE_VELOCITY, 0.25);
            setY(HEAD_UP_ANGLE_ANGLE_VELOCITY, 0.5);
        }
        state = FREE;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::fix()
{
    // Команда закрепить рукав на подвесе
    if (state == FREE)
        state = DO_FIX;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::unfix()
{
    // Команда снять рукав с подвеса
    if (state < FREE)
        state = FREE;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setExternalState(int command)
{
    // Воспроизводим логику Switcher для прямого управления с клавиатуры
    // Legacy
    if (pressed_keys && !pressed_keys->empty())
    {
        if ((getKeyState(*pressed_keys, key_symbol_dec) && isModifier(*pressed_keys, key_modifier_dec)))
        {
            command = 0;
        }
        else
        {
            if ((getKeyState(*pressed_keys, key_symbol_inc) && isModifier(*pressed_keys, key_modifier_inc)))
            {
                command = 2;
            }
        }
    }

    // Сохраняем команду: разъединить рукава, а если уже свободный - закрепить на подвесе
    if (command == 0)
    {
        command_disconnect_fix = true;

        command_unfix_connect = false;
        prev_command_unfix_connect = false;
        return;
    }

    // Сохраняем команду: снять рукав с подвеса, а если уже свободный - соединить
    if (command == 2)
    {
        command_unfix_connect = true;

        command_disconnect_fix = false;
        prev_command_disconnect_fix = false;
        return;
    }

    prev_command_disconnect_fix = false;
    prev_command_unfix_connect = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoHose::isConnected() const
{
    return (state == CONNECTED);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoHose::isFixed() const
{
    return (state == FIXED);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setPressure(double value)
{
    output_signals[HOSE_OUTPUT_PIPE_PRESSURE] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setFlowCoeff(double value)
{
    output_signals[HOSE_OUTPUT_FLOW_COEFF] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setMiddleLength(double value)
{
    output_signals[HOSE_OUTPUT_LENGTH_MIDDLE] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setHeadLength(double value)
{
    output_signals[HOSE_OUTPUT_LENGTH_HEAD] = value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setRootPosition(dvec3 position)
{
    root_pos = position;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setSideAngles(double existing, double free, double fixed)
{
    existing_side_angle = existing;
    free_side_angle = free;
    fixed_side_angle = fixed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setDownAngles(double existing, double free, double fixed)
{
    existing_down_angle = existing;
    free_down_angle = free;
    fixed_down_angle = fixed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setHeadUpAngles(double existing, double free, double fixed, double ready_to_connect)
{
    existing_head_up_angle = existing;
    free_head_up_angle = free;
    fixed_head_up_angle = fixed;
    ready_to_connect_head_up_angle = ready_to_connect;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::setVehiclePosition(const dvec3 position,
                                    const dvec3 vector_right,
                                    const dvec3 vector_orth,
                                    const dvec3 vector_up)
{
    v_right = vector_right;
    v_orth = vector_orth;
    v_up = vector_up;
    const dvec3 root_global_position =    position
                                        + v_right * root_pos.x
                                        + v_orth * root_pos.y
                                        + v_up * root_pos.z;
    output_signals[HOSE_OUTPUT_ROOT_COORD_X] = root_global_position.x;
    output_signals[HOSE_OUTPUT_ROOT_COORD_Y] = root_global_position.y;
    output_signals[HOSE_OUTPUT_ROOT_COORD_Z] = root_global_position.z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHose::getFlow() const
{
    if (is_linked)
        return input_signals[HOSE_INPUT_FLOW_TO_PIPE];

    // Если нет соседнего рукава, то рукав открыт в атмосферу
    return -( output_signals[HOSE_OUTPUT_FLOW_COEFF]
            * output_signals[HOSE_OUTPUT_PIPE_PRESSURE] );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHose::getSideAngle() const
{
    return getY(SIDE_ANGLE) - existing_side_angle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHose::getDownAngle() const
{
    return getY(DOWN_ANGLE) - existing_down_angle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PneumoHose::getHeadUpAngle() const
{
    return getY(HEAD_UP_ANGLE) - existing_head_up_angle;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
dvec3 PneumoHose::getRootPosition() const
{
    return root_pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
dvec3 PneumoHose::getConnectionPoint() const
{
    return v_hose;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
sound_state_t PneumoHose::getSoundState(size_t idx) const
{
    if (idx == PIPE_DRAIN_FLOW_SOUND)
        return atm_flow_sound;
    if (idx == CONNECT_SOUND)
        return sound_state_t(isConnected());
    if (idx == DISCONNECT_SOUND)
        return sound_state_t(!isConnected());
    return sound_state_t();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float PneumoHose::getSoundSignal(size_t idx) const
{
    if (idx == PIPE_DRAIN_FLOW_SOUND)
        return atm_flow_sound.createSoundSignal();
    if (idx == CONNECT_SOUND)
        return sound_state_t::createSoundSignal(isConnected());
    if (idx == DISCONNECT_SOUND)
        return sound_state_t::createSoundSignal(!isConnected());
    return sound_state_t::createSoundSignal(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::preStep(state_vector_t& Y, double t)
{
    // Выполняем команды управления

    if (    (input_signals[HOSE_INPUT_REF_CONNECTED] == 0.0)
         && (state == CONNECTED))
    {
        // Команда рассоединить рукава от модуля соединения
        disconnect();
    }

    if (    (input_signals[HOSE_INPUT_REF_CONNECTED] == 0.0)
         && (state == DO_CONNECT)
         && (output_signals[HOSE_OUTPUT_REF_STATE] == 1.0) )
    {
        // Модуль соединения не подтвердил возможность сцепить рукава,
        // отменяем команду на соединение
        disconnect();
    }

    if (command_disconnect_fix)
    {
        // Команда рассоединить рукава от пользователя
        if (!prev_command_disconnect_fix)
        {
            prev_command_disconnect_fix = true;
            (state > FREE) ? disconnect() : fix();
        }
    }
    else
    {
        // Команда соединить рукава из модуля соединения или от пользователя
        if (   (input_signals[HOSE_INPUT_REF_CONNECTED] == 1.0)
            || (command_unfix_connect && !prev_command_unfix_connect))
        {
            prev_command_unfix_connect = true;
            (state < FREE) ? unfix() : connect();
        }
    }

    // Сбрасываем пользовательские команды управления
    command_disconnect_fix = false;
    command_unfix_connect = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PneumoHose::check_angles_ready(double ref_side_angle,
                                    double ref_down_angle,
                                    double ref_head_up_angle)
{
    return (   (std::abs(ref_side_angle - getY(SIDE_ANGLE)) < 0.01)
            && (std::abs(ref_down_angle - getY(DOWN_ANGLE)) < 0.01)
            && (std::abs(ref_head_up_angle - getY(HEAD_UP_ANGLE)) < 0.01) );
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::ode_system(const state_vector_t &Y,
                           state_vector_t &dYdt,
                           double t)
{
    if (state <= FREE)
    {
        // Если рукав свободен, отправляем команду разъединить
        output_signals[HOSE_OUTPUT_REF_STATE] = -1.0;

        // Состояние никак не связано с соседним рукавом, а воздух свистит в атмосферу
        atm_flow_sound.state = true;
        atm_flow_sound.volume = K_sound * cbrt(getFlow());

        if (state == FREE)
        {
            // Рукав стремится к своим углам свободного состояния,
            // а углы свешивания считаем с инерцией, для раскачивания
            dYdt[SIDE_ANGLE] = (free_side_angle - Y[SIDE_ANGLE]) * angle_omega;
            dYdt[DOWN_ANGLE] = Y[DOWN_ANGLE_VELOCITY];
            dYdt[HEAD_UP_ANGLE] = Y[HEAD_UP_ANGLE_ANGLE_VELOCITY];
            dYdt[DOWN_ANGLE_VELOCITY] = free_angle_omega * (free_down_angle - Y[DOWN_ANGLE])
                                        - free_angle_beta * Y[DOWN_ANGLE_VELOCITY];
            dYdt[HEAD_UP_ANGLE_ANGLE_VELOCITY] = free_angle_omega * (free_head_up_angle - Y[HEAD_UP_ANGLE])
                                        - free_angle_beta * Y[HEAD_UP_ANGLE_ANGLE_VELOCITY];
            return;
        }

        if (state == DO_FIX)
        {
            // Проверяем, что рукав повернулся к подвесу
            if (check_angles_ready(fixed_side_angle, fixed_down_angle, fixed_head_up_angle))
            {
                // Закрепляем на подвесе
                state = FIXED;
                setY(SIDE_ANGLE, fixed_side_angle);
                setY(DOWN_ANGLE, fixed_down_angle);
                setY(HEAD_UP_ANGLE, fixed_head_up_angle);
                setY(DOWN_ANGLE_VELOCITY, 0.0);
                setY(HEAD_UP_ANGLE_ANGLE_VELOCITY, 0.0);

                dYdt[SIDE_ANGLE] = 0.0;
                dYdt[DOWN_ANGLE] = 0.0;
                dYdt[HEAD_UP_ANGLE] = 0.0;
                dYdt[DOWN_ANGLE_VELOCITY] = 0.0;
                dYdt[HEAD_UP_ANGLE_ANGLE_VELOCITY] = 0.0;
            }
            else
            {
                // Рукав стремится к своим углам для закрепления на подвесе
                dYdt[SIDE_ANGLE] = (fixed_side_angle - Y[SIDE_ANGLE]) * angle_omega;
                dYdt[DOWN_ANGLE] = (fixed_down_angle - Y[DOWN_ANGLE]) * angle_omega;
                dYdt[HEAD_UP_ANGLE] = (fixed_head_up_angle - Y[HEAD_UP_ANGLE]) * angle_omega;
                dYdt[DOWN_ANGLE_VELOCITY] = 0.0;
                dYdt[HEAD_UP_ANGLE_ANGLE_VELOCITY] = 0.0;
            }
        }
        return;
    }

    if (is_linked)
    {
        // Если должны быть соединены с соседним рукавом, считаем углы к точке соединения
        // Вектор к точке соединения
        dvec3 v = { input_signals[HOSE_INPUT_HEAD_ROOT_COORD_X] - output_signals[HOSE_OUTPUT_ROOT_COORD_X],
                    input_signals[HOSE_INPUT_HEAD_ROOT_COORD_Y] - output_signals[HOSE_OUTPUT_ROOT_COORD_Y],
                    input_signals[HOSE_INPUT_HEAD_ROOT_COORD_Z] - output_signals[HOSE_OUTPUT_ROOT_COORD_Z] };

        // Пересчитываем в локальные оси подвижной единицы
        v_hose =  { v_right.x * v.x + v_right.y * v.y + v_right.z * v.z,
                    v_orth.x  * v.x + v_orth.y  * v.y + v_orth.z  * v.z,
                    v_up.x    * v.x + v_up.y    * v.y + v_up.z    * v.z };
    }
    else
    {
        // Если мы получили команду соединить в отсутствие соседнего рукава,
        // просто поднимаем рукав в горизонталь, потом его всё равно сбросит
        // в свободное состояние
        v_hose = {output_signals[HOSE_OUTPUT_LENGTH_MIDDLE] * std::sin(existing_side_angle),
                  output_signals[HOSE_OUTPUT_LENGTH_MIDDLE] * std::cos(existing_side_angle),
                  0.0};
    }
    const dvec3 v_unit = normalize(v_hose);

    // Угол изгиба соединительной головки считается в модуле соединения
    double ref_head_up_angle = input_signals[HOSE_INPUT_HEAD_ROOT_ANGLE_UP];
    // Соединительные головки смыкаются не по оси рукава, нужно чуть развести их
    // Длина изогнутой части рукава в проекции на линию соединения
    const double len_mid = std::cos(ref_head_up_angle) * output_signals[HOSE_OUTPUT_LENGTH_MIDDLE];
    // Добавка к углу, чтобы развести головки на 1 сантиметр влево
    const double gap_angle = std::asin(side_gap / (len_mid + output_signals[HOSE_OUTPUT_LENGTH_HEAD]));
    // Углы к точке соединения
    const double ref_side_angle = std::atan2(v_unit.x, v_unit.y) + gap_angle;
    const double ref_down_angle = std::asin(v_unit.z);

    if (state == DO_CONNECT)
    {
        if (input_signals[HOSE_INPUT_IS_CONNECTED] == 1.0)
        {
            // Модуль соединения сообщил об успешном соединении рукавов
            state = CONNECTED;

            // Сбрасываем управляющий сигнал
            output_signals[HOSE_OUTPUT_REF_STATE] = 0.0;
        }
        else
        {
            // Соединительные головки нужно повернуть вверх чтобы соединить
            ref_head_up_angle += ready_to_connect_head_up_angle;

            if (check_angles_ready(ref_side_angle, ref_down_angle, ref_head_up_angle))
            {
                // Все углы повернулись, отправляем команду соединить
                output_signals[HOSE_OUTPUT_REF_STATE] = 1.0;
            }
            else
            {
                // Рукав стремится к своим углам для соединения с соседним
                dYdt[SIDE_ANGLE] = (ref_side_angle - Y[SIDE_ANGLE]) * angle_omega;
                dYdt[DOWN_ANGLE] = (ref_down_angle - Y[DOWN_ANGLE]) * angle_omega;
                dYdt[HEAD_UP_ANGLE] = (ref_head_up_angle - Y[HEAD_UP_ANGLE]) * angle_omega;
                dYdt[DOWN_ANGLE_VELOCITY] = 0.0;
                dYdt[HEAD_UP_ANGLE_ANGLE_VELOCITY] = 0.0;
                return;
            }
        }
    }

    // Рукав следит за точкой соединения с соседним
    setY(SIDE_ANGLE, ref_side_angle);
    setY(DOWN_ANGLE, ref_down_angle);
    setY(HEAD_UP_ANGLE, ref_head_up_angle);
    setY(DOWN_ANGLE_VELOCITY, 0.0);
    setY(HEAD_UP_ANGLE_ANGLE_VELOCITY, 0.0);

    dYdt[SIDE_ANGLE] = 0.0;
    dYdt[DOWN_ANGLE] = 0.0;
    dYdt[HEAD_UP_ANGLE] = 0.0;
    dYdt[DOWN_ANGLE_VELOCITY] = 0.0;
    dYdt[HEAD_UP_ANGLE_ANGLE_VELOCITY] = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PneumoHose::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);

    cfg.getDouble(secName, "K_sound", K_sound);


    QString pos = "0.0 0.0 0.0";
    if (cfg.getString(secName, "RootPosition", pos))
    {
        std::istringstream stream(pos.toStdString());
        stream  >> root_pos.x
                >> root_pos.y
                >> root_pos.z;
    }

    // Углы задаются в конфигурационном файле в градусах, пересчитываем в радианы
    double tmp;

    // Длина средней части рукава (от точки изгиба в начале
    // до точки изгиба перед соединительной головкой), м
    if (cfg.getDouble(secName, "MiddleLength", tmp))
        setMiddleLength(tmp);

    // Несовпадение поверхности смыкания соединительной головки с осью рукава, м
    if (cfg.getDouble(secName, "HeadLength", tmp))
        setHeadLength(tmp);


    // Существующее конструктивное отклонение рукава от продольной оси
    if (cfg.getDouble(secName, "ExistingSideAngle", tmp))
        existing_side_angle = tmp / 180.0 * Physics::PI;

    // Существующее конструктивное отклонение рукава от продольной оси
    if (cfg.getDouble(secName, "ExistingSideAngle", tmp))
        existing_side_angle = tmp / 180.0 * Physics::PI;

    // Отклонение рукава от продольной оси в свободном состоянии
    if (cfg.getDouble(secName, "FreeSideAngle", tmp))
        free_side_angle = tmp / 180.0 * Physics::PI;

    // Отклонение рукава от продольной оси в закреплённом на подвесе состоянии
    if (cfg.getDouble(secName, "FixedSideAngle", tmp))
        fixed_side_angle = tmp / 180.0 * Physics::PI;

    // Несовпадение поверхности смыкания соединительной головки с осью рукава, м
    cfg.getDouble(secName, "SideGap", side_gap);


    // Существующее конструктивное свешивание рукава вниз от продольной оси
    if (cfg.getDouble(secName, "ExistingDownAngle", tmp))
        existing_down_angle = tmp / 180.0 * Physics::PI;

    // Свешивание рукава вниз от продольной оси в свободном состоянии
    if (cfg.getDouble(secName, "FreeDownAngle", tmp))
        free_down_angle = tmp / 180.0 * Physics::PI;

    // Свешивание рукава вниз от продольной оси в закреплённом на подвесе состоянии
    if (cfg.getDouble(secName, "FixedDownAngle", tmp))
        fixed_down_angle = tmp / 180.0 * Physics::PI;


    // Существующий конструктивный изгиб вверх соединительной головки рукава
    if (cfg.getDouble(secName, "ExistingHeadUpAngle", tmp))
        existing_head_up_angle = tmp / 180.0 * Physics::PI;

    // Изгиб рукава вверх перед соединительной головкой в свободном состоянии
    if (cfg.getDouble(secName, "FreeHeadUpAngle", tmp))
        free_head_up_angle = tmp / 180.0 * Physics::PI;

    // Изгиб рукава вверх перед соединительной головкой в закреплённом на подвесе состоянии
    if (cfg.getDouble(secName, "FixedHeadUpAngle", tmp))
        fixed_head_up_angle = tmp / 180.0 * Physics::PI;

    // Изгиб рукава вверх перед соединительной головкой в состоянии готовности к соединению
    if (cfg.getDouble(secName, "ReadyToConnectHeadUpAngle", tmp))
        ready_to_connect_head_up_angle = tmp / 180.0 * Physics::PI;


    // Коэффициент к скорости поворотов рукава
    cfg.getDouble(secName, "AngleOmega", angle_omega);

    // Коэффициент к скорости колебаний свешивания рукава в свободном состоянии
    cfg.getDouble(secName, "FreeAngleOmega", free_angle_omega);

    // Коэффициент затухания колебаний свешивания рукава в свободном состоянии
    cfg.getDouble(secName, "FreeAngleBeta", free_angle_beta);
}
