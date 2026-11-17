#ifndef     VEHICLE_CONTROL_SIGNALS_CONTAINER_H
#define     VEHICLE_CONTROL_SIGNALS_CONTAINER_H

#include    <QMap>
#include    <vector>
#include    <vehicle-control-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
// Контейнер управляющих сигналов, принимаемых от IOController.
// Обеспечивает безопасную работу с сигналами каждого транспортного средства,
// хранящимися по кабинам. Последняя (общая) кабина предназначена для сигналов,
// не привязанных к конкретной кабине.
//------------------------------------------------------------------------------
class VehiclesControlSignalsContainer
{
public:

    typedef std::vector<QMap<int, vehicle_control_signal_t>>  CabineInputs;
    typedef CabineInputs::iterator                            iterator;
    typedef CabineInputs::const_iterator                      const_iterator;

    VehiclesControlSignalsContainer() = default;

    ~VehiclesControlSignalsContainer() = default;

    // Задаёт количество кабин (создаёт записи для каждой, включая общую)
    void setCabinesNumber(const size_t &cabs_num)
    {
        control_inputs.resize(cabs_num);
    }

    // Добавляет сигнал в указанную кабину
    void addInput(const size_t &cab_idx, const int &id, const vehicle_control_signal_t &input)
    {
        if (cab_idx >= control_inputs.size())
        {
            return;
        }

        control_inputs[cab_idx].insert(id, input);
    }

    // Возвращает копию сигнала по индексу кабины и идентификатору
    vehicle_control_signal_t getInput(const size_t &cab_idx, const int &id)
    {
        if (cab_idx >= control_inputs.size())
        {
            return vehicle_control_signal_t();
        }

        return control_inputs[cab_idx].value(id, vehicle_control_signal_t());
    }

    // Изменяет значение существующего сигнала в указанной кабине
    void setInput(const size_t &cab_idx, const int &id, const vehicle_control_signal_t &value)
    {
        if (cab_idx >= control_inputs.size())
        {
            return;
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it != control_inputs[cab_idx].end())
        {
            it.value() = value;
        }
    }

    // Доступ к карте сигналов указанной кабины (для оператора[] сигнала)
    QMap<int, vehicle_control_signal_t>& operator[](const size_t &cab_idx)
    {
        return control_inputs[cab_idx];
    }

    // Константный доступ к карте сигналов указанной кабины
    const QMap<int, vehicle_control_signal_t>& operator[](const size_t &cab_idx) const
    {
        return control_inputs[cab_idx];
    }

    // Количество кабин
    size_t size() const
    {
        return control_inputs.size();
    }

    // Признак отсутствия кабин
    bool empty() const
    {
        return control_inputs.empty();
    }

    iterator begin()
    {
        return control_inputs.begin();
    }

    iterator end()
    {
        return control_inputs.end();
    }

    const_iterator begin() const
    {
        return control_inputs.begin();
    }

    const_iterator end() const
    {
        return control_inputs.end();
    }

private:

    // Хранилище сигналов по кабинам
    CabineInputs control_inputs;
};

#endif
