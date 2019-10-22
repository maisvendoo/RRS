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
        // Передаем значение дискретных входов
        for (slave_data_t data : slave->discrete_input)
        {
            control_signals.analogSignal[data.index].is_active = true;
            control_signals.analogSignal[data.index].value = static_cast<float>(data.value);
        }

        // Передаем значения регистров ввода
        for (slave_data_t data : slave->input_register)
        {
            control_signals.analogSignal[data.index].is_active = true;
            control_signals.analogSignal[data.index].value = static_cast<float>(data.value);
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Modbus::feedbackSignalsProcess()
{
    for (Slave *slave: master->slave)
    {
        QMap<quint16, slave_data_t>::iterator it;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
slave_data_t *Modbus::searchByIndex(size_t index, data_map_t &data)
{
    QMap<quint16, slave_data_t>::iterator it;

    for (it = data.begin(); it != data.end(); ++it)
    {
        if (it.value().index == index)
        {
            return &it.value();
        }
    }

    return nullptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_INTERFACE_DEVICE(Modbus)
