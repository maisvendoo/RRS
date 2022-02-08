#include    "modbus.h"

#include    "slave-data.h"

#include    <QDebug>

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
    try
    {
        master = new Master();

    } catch (const std::bad_alloc &)
    {
        return false;
    }

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
            control_signals.analogSignal[data.index].cur_value = static_cast<float>(data.cur_value);
        }

        master->readInputRegistersRequest(slave);
        // Передаем значения регистров ввода
        for (slave_data_t data : slave->input_register)
        {
            control_signals.analogSignal[data.index].is_active = true;
            control_signals.analogSignal[data.index].cur_value = static_cast<float>(data.cur_value);
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
        bool is_changed = false;

        for (it = slave->coil.begin(); it != slave->coil.end(); ++it)
        {
            slave_data_t *coil = &it.value();
            coil->setValue(static_cast<quint16>(feedback_signals.analogSignal[it.value().index].cur_value));

            is_changed = is_changed || coil->isChanged();
        }

        if (is_changed)
            master->writeCoils(slave);

        // Пишем регистры вывода
        is_changed = false;

        for (it = slave->holding_register.begin(); it != slave->holding_register.end(); ++it)
        {
            slave_data_t *holding_reg = &it.value();
            holding_reg->setValue(static_cast<quint16>(feedback_signals.analogSignal[it.value().index].cur_value));

            is_changed = is_changed || holding_reg->isChanged();
        }        

        if (is_changed)
        {
            qDebug() << feedback_signals.analogSignal[29].cur_value;
            master->writeHoldingRegisters(slave);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_INTERFACE_DEVICE(Modbus)
