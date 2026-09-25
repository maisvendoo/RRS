#ifndef     TOGGLE_HANDLER_H
#define     TOGGLE_HANDLER_H

#include    <control-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ToggleHandler : public ControlHandler
{
public:

    explicit ToggleHandler(QObject *parent = nullptr);

    void processKeyInput(const std::set<uint16_t>& pressed_keys) override;

    void processMouseInput(uint32_t button, bool is_pressed) override;

private:

    void processTumbler(size_t cab_idx, uint16_t control_id,
                       const std::set<uint16_t>& pressed_keys);

    void processButton(size_t cab_idx, uint16_t control_id,
                       const std::set<uint16_t>& pressed_keys);
};

#endif
