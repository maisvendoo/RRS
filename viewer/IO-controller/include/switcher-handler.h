#ifndef     SWITCHER_HANDLER_H
#define     SWITCHER_HANDLER_H

#include    <control-handler.h>

//------------------------------------------------------------------------------
// SwitcherHandler — многопозиционный переключатель.
// Поддерживает автоповтор при удержании и пружинный
// возврат из крайних положений.
//
// Позиции задаются через NumPositions, MinValue, MaxValue. Клавиши инкремента
// и декремента — KeyNameInc / KeyNameDec (с опциональными модификаторами).
// SpringReturnLow / SpringReturnHigh — номера позиций, с которых делать
// автоматический возврат при отпускании клавиши/кнопки.
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

    uint16_t keyCodeInc = 0;          // клавиша увеличения позиции
    QString  keyModIncName = "";      // модификатор увеличения (опционально)
    uint16_t keyCodeDec = 0;          // клавиша уменьшения позиции
    QString  keyModDecName = "";      // модификатор уменьшения (опционально)
    uint16_t numPositions = 2;        // количество позиций
    float minValue = 0.0f;            // минимальное значение сигнала
    float maxValue = 1.0f;            // максимальное значение сигнала
    /// Автоматический возврат на одну позицию внутрь при отпускании
    /// клавиши/кнопки на крайней позиции. -1 = отключено.
    int springReturnLow = -1;         // возврат с нижней (0-й) позиции → +1
    int springReturnHigh = -1;        // возврат с верхней позиции → -1

    int    hold_direction = 0;        // направление удержания (+1/-1/0)
    float  hold_time = 0.0f;          // время удержания для автоповтора
    bool   spring_low_triggered = false;   // флаг: возврат с нижней уже был
    bool   spring_high_triggered = false;  // флаг: возврат с верхней уже был

    static constexpr float HOLD_DELAY      = 0.3f;   // задержка перед автоповтором
    static constexpr float REPEAT_INTERVAL = 0.1f;   // интервал автоповтора

    int currentIndex() const;
    void sendNextPosition(int direction);
    void doSpringReturn();
};

#endif
