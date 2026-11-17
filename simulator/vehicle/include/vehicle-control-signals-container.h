#ifndef     VEHICLE_CONTROL_SIGNALS_CONTAINER_H
#define     VEHICLE_CONTROL_SIGNALS_CONTAINER_H

#include    <QMap>
#include    <vector>
#include    <control-priority.h>
#include    <vehicle-control-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
// Контейнер управляющих сигналов, принимаемых от IOController.
// Инкапсулирует хранилище и предоставляет безопасный (с проверкой границ)
// доступ к сигналам каждого транспортного средства, хранящимся по кабинам.
// Последняя (общая) кабина (индекс size()-1) предназначена для сигналов,
// не привязанных к конкретной кабине.
//------------------------------------------------------------------------------
class VehiclesControlSignalsContainer
{
public:

    VehiclesControlSignalsContainer() = default;

    ~VehiclesControlSignalsContainer() = default;

    // Задаёт количество кабин (создаёт запись для каждой, включая общую)
    void setCabinesNumber(const size_t &cabs_num)
    {
        control_inputs.resize(cabs_num);
    }

    // Добавляет сигнал в указанную кабину (дополняет существующую запись)
    void addInput(const size_t &cab_idx, const int &id, const vehicle_control_signal_t &input)
    {
        if (cab_idx >= control_inputs.size())
        {
            return;
        }

        control_inputs[cab_idx].insert(id, input);
    }

    // Признак существования сигнала в указанной кабине
    bool contains(const size_t &cab_idx, const int &id) const
    {
        if (cab_idx >= control_inputs.size())
        {
            return false;
        }

        return control_inputs[cab_idx].contains(id);
    }

    // Значение сигнала как флаг (value != 0)
    bool getBool(const size_t &cab_idx, const int &id) const
    {
        if (cab_idx >= control_inputs.size())
        {
            return false;
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it == control_inputs[cab_idx].end())
        {
            return false;
        }

        return it.value().value != 0.0f;
    }

    // Числовое значение сигнала
    float getValue(const size_t &cab_idx, const int &id) const
    {
        if (cab_idx >= control_inputs.size())
        {
            return 0.0f;
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it == control_inputs[cab_idx].end())
        {
            return 0.0f;
        }

        return it.value().value;
    }

    // Устанавливает числовое значение сигнала в указанной кабине
    void setValue(const size_t &cab_idx, const int &id, const float &value)
    {
        if (cab_idx >= control_inputs.size())
        {
            return;
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it != control_inputs[cab_idx].end())
        {
            it.value().value = value;
        }
    }

    // Приоритет сервера для сигнала указанной кабины
    int getPriority(const size_t &cab_idx, const int &id) const
    {
        if (cab_idx >= control_inputs.size())
        {
            return static_cast<int>(CTRL_PRIORITY_FULL);
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it == control_inputs[cab_idx].end())
        {
            return static_cast<int>(CTRL_PRIORITY_FULL);
        }

        return static_cast<int>(it.value().server_priority);
    }

    // Устанавливает приоритет сервера для сигнала указанной кабины
    void setPriority(const size_t &cab_idx, const int &id, const ControlPriority &priority)
    {
        if (cab_idx >= control_inputs.size())
        {
            return;
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it != control_inputs[cab_idx].end())
        {
            it.value().server_priority = priority;
        }
    }

    // Количество параметров сигнала указанной кабины
    size_t getParamsSize(const size_t &cab_idx, const int &id) const
    {
        if (cab_idx >= control_inputs.size())
        {
            return 0;
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it == control_inputs[cab_idx].end())
        {
            return 0;
        }

        return it.value().params.size();
    }

    // Значение параметра сигнала указанной кабины
    float getParam(const size_t &cab_idx, const int &id, const size_t &index) const
    {
        if (cab_idx >= control_inputs.size())
        {
            return 0.0f;
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it == control_inputs[cab_idx].end())
        {
            return 0.0f;
        }

        if (index >= it.value().params.size())
        {
            return 0.0f;
        }

        return it.value().params[index];
    }

    // Устанавливает параметр сигнала указанной кабины
    void setParam(const size_t &cab_idx, const int &id, const size_t &index, const float &value)
    {
        if (cab_idx >= control_inputs.size())
        {
            return;
        }

        auto it = control_inputs[cab_idx].find(id);

        if (it != control_inputs[cab_idx].end() && index < it.value().params.size())
        {
            it.value().params[index] = value;
        }
    }

    // Устанавливает приоритет сервера для всех сигналов всех кабин
    void setAllPriority(const ControlPriority &priority)
    {
        for (auto &cab : control_inputs)
        {
            for (auto it = cab.begin(); it != cab.end(); ++it)
            {
                it.value().server_priority = priority;
            }
        }
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

private:

    // Хранилище сигналов по кабинам
    std::vector<QMap<int, vehicle_control_signal_t>> control_inputs;
};

#endif
