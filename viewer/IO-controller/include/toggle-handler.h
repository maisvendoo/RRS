#ifndef     TOGGLE_HANDLER_H
#define     TOGGLE_HANDLER_H

#include    <bistate-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IO_CONTROLLER_EXPORT ToggleHandler : public BiStateHandler
{
public:

    ToggleHandler(QObject *parent = nullptr);

    virtual void processKeyInput(const std::set<uint16_t>& pressed_keys) override;

    virtual void processMouseInput(uint32_t button, bool is_pressed) override;

    virtual QString getUsage() const override;

    virtual QString getState() const override;

    virtual bool load_config(CfgReader &cfg, QDomNode secNode) override;

protected:

    void processTumbler(size_t cab_idx, uint16_t control_id,
                       const std::set<uint16_t>& pressed_keys);

    void processButton(size_t cab_idx, uint16_t control_id,
                       const std::set<uint16_t>& pressed_keys);

    QString onButton = "LEFT_BUTTON";
    QString offButton = "RIGHT_BUTTON";
};

#endif
