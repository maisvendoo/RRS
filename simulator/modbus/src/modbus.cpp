#include    "modbus.h"

#include    "slave-data.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Modbus::Modbus(QObject *parent) : VirtualInterfaceDevice(parent)
  , master(Q_NULLPTR)
  , is_transmit(false)
{
    cfg_dir = "modbus";
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Modbus::~Modbus()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Modbus::init(QString cfg_path)
{
    master = new Master();

    if (master == Q_NULLPTR)
        return false;

    if (!master->init(cfg_path))
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Modbus::process()
{
    // Разделяем прием и передачу во времени, дабы не блокировать шину
    // ибо полудуплекс
    if (is_transmit)
    {
        feedbackSignalsProcess();
    }
    else
    {
        controlSignalsProcess();
    }

    // Переключаемся с приема на передачу
    is_transmit = !is_transmit;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Modbus::controlSignalsProcess()
{
    // Перебираем все ведомые устройства
    for (Slave *slave : master->slave)
    {
        // Послыаем запрос на чтение дискретных входов
        master->readDiscreteInputsRequest(slave);
        // Передаем значение дискретных входов        
        for (slave_data_t data : slave->discrete_input)
        {
            control_signals.analogSignal[data.index].is_active = true;
            control_signals.analogSignal[data.index].setValue(static_cast<float>(data.cur_value));
        }

        master->readInputRegistersRequest(slave);
        // Передаем значения регистров ввода
        for (slave_data_t data : slave->input_register)
        {
            control_signals.analogSignal[data.index].is_active = true;
            control_signals.analogSignal[data.index].setValue(static_cast<float>(data.cur_value));
        }
    }

    emit sendControlSignals(control_signals);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Modbus::feedbackSignalsProcess()
{
    for (Slave *slave: master->slave)
    {
        QMap<quint16, slave_data_t>::iterator it;

        // Пишем дискретные выходы
        for (it = slave->coil.begin(); it != slave->coil.end(); ++it)
        {
            slave_data_t *coil = &it.value();
            coil->cur_value = static_cast<quint16>(feedback_signals.analogSignal[it.value().index].cur_value);
        }

        // Отправляем их в шину
        master->writeCoils(slave);

        // Пишем регистры вывода
        for (it = slave->holding_register.begin(); it != slave->holding_register.end(); ++it)
        {
            slave_data_t *holding_reg = &it.value();
            holding_reg->cur_value = static_cast<quint16>(feedback_signals.analogSignal[it.value().index].cur_value);

            if (holding_reg->cur_value != holding_reg->prev_value)
            {
                master->writeHoldingRegister(slave, *holding_reg);
                holding_reg->prev_value = holding_reg->cur_value;
            }
        }

        // Шлем в шину
        //master->writeHoldingRegisters(slave);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_INTERFACE_DEVICE(Modbus)
