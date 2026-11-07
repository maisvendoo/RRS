#ifndef     AXIS_HANDLER_H
#define     AXIS_HANDLER_H

#include    <range-handler.h>

//------------------------------------------------------------------------------
// AxisHandler — аналоговая ось. Не имеет фиксированных позиций.
// При удержании inc-клавиши/кнопки значение плавно увеличивается,
// при удержании dec-клавиши/кнопки — уменьшается.
// Скорость изменения задаётся в конфиге (Speed, ед/с).
// SpringReturnLow / SpringReturnHigh — пороги: при отпускании,
// если значение пересекло порог, оно возвращается к нему.
//------------------------------------------------------------------------------
class IO_CONTROLLER_EXPORT AxisHandler : public RangeHandler
{
public:

    AxisHandler(QObject *parent = nullptr);

    virtual bool load_config(CfgReader &cfg, QDomNode secNode) override;

    virtual void processKeyInput(const std::set<uint16_t> &pressed_keys) override;

    virtual void processMouseInput(uint32_t button, bool is_pressed) override;

    virtual void step(float t, float dt) override;

    virtual QString getState() const override;

protected:

    float minValue = 0.0f;            // минимальное значение сигнала
    float maxValue = 1.0f;            // максимальное значение сигнала
    float speed = 0.5f;               // скорость изменения, ед/с
    float springReturnLow = -1.0f;    // нижний порог возврата (-1 = отключен)
    float springReturnHigh = -1.0f;   // верхний порог возврата (-1 = отключен)

    int   hold_direction = 0;         // направление удержания (+1/-1/0)    

    void doSpringReturn();
};

#endif
