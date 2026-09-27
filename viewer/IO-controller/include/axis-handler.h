#ifndef     AXIS_HANDLER_H
#define     AXIS_HANDLER_H

#include    <control-handler.h>

//------------------------------------------------------------------------------
// AxisHandler — аналоговая ось. Не имеет фиксированных позиций.
// При удержании inc-клавиши/кнопки значение плавно увеличивается,
// при удержании dec-клавиши/кнопки — уменьшается.
// Скорость изменения задаётся в конфиге (Speed, ед/с).
// SpringReturnLow / SpringReturnHigh — пороги: при отпускании,
// если значение пересекло порог, оно возвращается к нему.
//------------------------------------------------------------------------------
class AxisHandler : public ControlHandler
{
public:

    explicit AxisHandler(QObject *parent = nullptr);

    bool load_config(CfgReader &cfg, QDomNode secNode) override;

    void processKeyInput(const std::set<uint16_t> &pressed_keys) override;

    void processMouseInput(uint32_t button, bool is_pressed) override;

    void step(float t, float dt) override;

    QString getUsage() const override;

    QString getState() const override;

    uint16_t keyCodeInc = 0;          // клавиша увеличения
    QString  keyModIncName = "";      // модификатор увеличения
    uint16_t keyCodeDec = 0;          // клавиша уменьшения
    QString  keyModDecName = "";      // модификатор уменьшения

private:

    float minValue = 0.0f;            // минимальное значение сигнала
    float maxValue = 1.0f;            // максимальное значение сигнала
    float speed = 0.5f;               // скорость изменения, ед/с
    float springReturnLow = -1.0f;    // нижний порог возврата (-1 = отключен)
    float springReturnHigh = -1.0f;   // верхний порог возврата (-1 = отключен)

    int   hold_direction = 0;         // направление удержания (+1/-1/0)

    QString incButtonName = "LEFT_BUTTON";
    QString decButtonName = "RIGHT_BUTTON";

    uint16_t resetKey = KEY_Undefined;
    QString  resetModkey = "";
    float    resetValue = 0.0f;

    void doSpringReturn();
};

#endif