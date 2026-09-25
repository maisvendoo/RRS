#ifndef     BUTTON_HANDLER_H
#define     BUTTON_HANDLER_H

#include    <control-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ButtonHandler : public ControlHandler
{
public:

    explicit ButtonHandler(QObject *parent = nullptr);

    void processKeyInput(const std::set<uint16_t>& pressed_keys,
                         int cabine_idx, int vehicle_idx) override;

    void processMouseInput(const io_control_input_t& input,
                           uint32_t button, bool is_pressed) override;
};

#endif
