#ifndef     SWITCHER_HANDLER_H
#define     SWITCHER_HANDLER_H

#include    <control-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SwitcherHandler : public ControlHandler
{
public:

    explicit SwitcherHandler(QObject *parent = nullptr);

    bool load_config(CfgReader &cfg, QDomNode secNode) override;

    void processKeyInput(const std::set<uint16_t> &pressed_keys) override;

    void processMouseInput(uint32_t button, bool is_pressed) override;

    void step(float t, float dt) override;

    QString getUsage() const override;

private:

    uint16_t keyCodeInc = 0;
    QString  keyModIncName = "";
    uint16_t keyCodeDec = 0;
    QString  keyModDecName = "";
    uint16_t numPositions = 2;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    int springReturnLow = -1;
    int springReturnHigh = -1;

    int    hold_direction = 0;
    float  hold_time = 0.0f;
    bool   spring_low_triggered = false;
    bool   spring_high_triggered = false;

    static constexpr float HOLD_DELAY      = 0.3f;
    static constexpr float REPEAT_INTERVAL = 0.1f;

    int currentIndex() const;
    void sendNextPosition(int direction);
};

#endif