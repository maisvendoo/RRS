#ifndef     BUTTON_HANDLER_H
#define     BUTTON_HANDLER_H

#include    <bistate-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IO_CONTROLLER_EXPORT ButtonHandler : public BiStateHandler
{
public:

    ButtonHandler(QObject *parent = nullptr);

    virtual void processKeyInput(const std::set<uint16_t>& pressed_keys) override;

    virtual void processMouseInput(uint32_t button, bool is_pressed) override;

    virtual QString getUsage() const override;

    virtual QString getState() const override;

    virtual bool load_config(CfgReader &cfg, QDomNode secNode) override;

    virtual void reset() override;

protected:

    /// Разрешать удержание кнопки
    bool allow_hold = false;

    /// Флаг того, что кнопка в режиме удержания
    bool is_holded = false;

    QString pressButton = "LEFT_BUTTON";
    QString holdButton = "RIGHT_BUTTON";
};

#endif
