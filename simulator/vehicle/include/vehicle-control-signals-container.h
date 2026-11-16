#ifndef     VEHICLE_CONTROL_SIGNALS_CONTAINER_H
#define     VEHICLE_CONTROL_SIGNALS_CONTAINER_H

#include    <QMap>
#include    <vehicle-control-signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VehiclesControlSignalsContainer
{
public:

    VehiclesControlSignalsContainer() = default;

    ~VehiclesControlSignalsContainer() = default;

    void setCabinesNumber(const size_t &cabs_num)
    {
        control_inputs.clear();
        control_inputs.reserve(cabs_num);
    }

    void addInput(const size_t &cab_idx, const int &id, const vehicle_control_signal_t &input)
    {
        if (cab_idx >= control_inputs.size())
        {
            return;
        }

        control_inputs[cab_idx].insert(id, input);
    }

    vehicle_control_signal_t getInput(const size_t &cab_idx, const int &id)
    {
        if (cab_idx >= control_inputs.size())
        {
            return vehicle_control_signal_t();
        }

        return control_inputs[cab_idx].value(id, vehicle_control_signal_t());
    }

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

private:

    std::vector<QMap<int, vehicle_control_signal_t>> control_inputs;
};

#endif
