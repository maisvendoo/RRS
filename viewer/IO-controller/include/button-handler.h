#ifndef     BUTTON_HANDLER_H
#define     BUTTON_HANDLER_H

#include    <bistate-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ButtonHandler : public BiStateHandler
{
public:

    explicit ButtonHandler(QObject *parent = nullptr);

    void processKeyInput(const std::set<uint16_t>& pressed_keys) override;

    void processMouseInput(uint32_t button, bool is_pressed) override;

    QString getUsage() const override;

    QString getState() const override;

    bool load_config(CfgReader &cfg, QDomNode secNode) override;

    void reset() override;

private:

    /// Разрешать удержание кнопки
    bool allow_hold = false;

    /// Флаг того, что кнопка в режиме удержания
    bool is_holded = false;

    QString pressButton = "LEFT_BUTTON";
    QString holdButton = "RIGHT_BUTTON";
};

#endif
