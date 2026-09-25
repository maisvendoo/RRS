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

    void processKeyInput(const std::set<uint16_t>& pressed_keys) override;

    void processMouseInput(uint32_t button, bool is_pressed) override;

    QString getUsage() const override;
};

#endif
