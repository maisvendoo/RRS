#ifndef     TOGGLE_HANDLER_H
#define     TOGGLE_HANDLER_H

#include    <control-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ToggleHandler : public ControlHandler
{
public:

    ToggleHandler(QObject *parent = nullptr);

    void processKeyInput(const std::set<uint16_t>& pressed_keys,
                         int cabine_idx, int vehicle_idx) override;

    void processMouseInput(const io_control_input_t& input,
                           uint32_t button, bool is_pressed) override;

private:

    void processTumbler(size_t cab_idx, uint16_t control_id,
                       const std::set<uint16_t>& pressed_keys);

    void processButton(size_t cab_idx, uint16_t control_id,
                       const std::set<uint16_t>& pressed_keys);
};

#endif
