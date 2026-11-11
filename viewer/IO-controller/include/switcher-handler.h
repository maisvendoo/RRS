#ifndef     SWITCHER_HANDLER_H
#define     SWITCHER_HANDLER_H

#include    <range-handler.h>

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
class IO_CONTROLLER_EXPORT SwitcherHandler : public RangeHandler
{
public:

    SwitcherHandler(QObject *parent = nullptr);

    virtual bool load_config(CfgReader &cfg, QDomNode secNode) override;

    virtual void processKeyInput(const std::set<uint16_t> &pressed_keys) override;

    virtual void processMouseInput(uint32_t button, bool is_pressed) override;

    virtual void step(float t, float dt) override;

    virtual QString getState() const override;

protected:

    uint16_t numPositions = 2;        // количество позиций
    float minValue = 0.0f;            // минимальное значение сигнала
    float maxValue = 1.0f;            // максимальное значение сигнала
    float range = 1.0f;
    float val_step = 0.0f;
    /// Автоматический возврат на одну позицию внутрь при отпускании
    /// клавиши/кнопки на крайней позиции. -1 = отключено.
    int springReturnLow = -1;         // возврат с нижней (0-й) позиции → +1
    int springReturnHigh = -1;        // возврат с верхней позиции → -1

    int    hold_direction = 0;        // направление удержания (+1/-1/0)
    float  hold_time = 0.0f;          // время удержания для автоповтора
    bool   spring_low_triggered = false;   // флаг: возврат с нижней уже был
    bool   spring_high_triggered = false;  // флаг: возврат с верхней уже был
    bool   prev_reset_key = false;    // защита: клавиша сброса не срабатывает как inc/dec

    /// Имена позиций, для отображения в статусе
    QStringList positionNames;     

    static constexpr float HOLD_DELAY      = 0.3f;   // задержка перед автоповтором
    static constexpr float REPEAT_INTERVAL = 0.1f;   // интервал автоповтора

    int currentIndex() const;
    void sendNextPosition(int direction);
    void doSpringReturn();
};

#endif
