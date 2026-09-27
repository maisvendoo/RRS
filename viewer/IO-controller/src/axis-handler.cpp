#include    <axis-handler.h>

#include    <io-controller-keymap.h>
#include    <CfgReader.h>

#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AxisHandler::AxisHandler(QObject *parent) : RangeHandler(parent)
{

}

//------------------------------------------------------------------------------
// Загрузка конфигурации аналоговой оси
//------------------------------------------------------------------------------
bool AxisHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    ControlHandler::load_config(cfg, secNode);

    // Клавиша увеличения
    QString keyInc;
    cfg.getString(secNode, "KeyNameInc", keyInc);
    keyCodeInc = KeySymbolsRRSMap.value(keyInc, KEY_Undefined);
    cfg.getString(secNode, "KeyModIncName", keyModIncName);

    // Клавиша уменьшения
    QString keyDec;
    cfg.getString(secNode, "KeyNameDec", keyDec);
    keyCodeDec = KeySymbolsRRSMap.value(keyDec, KEY_Undefined);
    cfg.getString(secNode, "KeyModDecName", keyModDecName);

    // Диапазон
    double tmp_min = 0.0, tmp_max = 1.0;
    cfg.getDouble(secNode, "MinValue", tmp_min);
    cfg.getDouble(secNode, "MaxValue", tmp_max);
    minValue = static_cast<float>(tmp_min);
    maxValue = static_cast<float>(tmp_max);

    // Скорость
    double tmp_speed = 0.5;
    cfg.getDouble(secNode, "Speed", tmp_speed);
    speed = static_cast<float>(tmp_speed);

    // Кнопки мыши
    cfg.getString(secNode, "IncButtonName", incButtonName);
    cfg.getString(secNode, "DecButtonName", decButtonName);

    // Пороги пружинного возврата
    double tmp_sr = -1.0;
    cfg.getDouble(secNode, "SpringReturnLow", tmp_sr);
    springReturnLow = static_cast<float>(tmp_sr);

    tmp_sr = -1.0;
    cfg.getDouble(secNode, "SpringReturnHigh", tmp_sr);
    springReturnHigh = static_cast<float>(tmp_sr);

    // Клавиша сброса в заданное положение
    QString resetKeyName = "";
    cfg.getString(secNode, "ResetKey", resetKeyName);
    resetKey = KeySymbolsRRSMap.value(resetKeyName, KEY_Undefined);
    cfg.getString(secNode, "ResetModkey", resetModkey);

    double tmp_rv = 0.0;
    cfg.getDouble(secNode, "ResetValue", tmp_rv);
    resetValue = static_cast<float>(tmp_rv);

    return true;
}

//------------------------------------------------------------------------------
// Пружинный возврат: при отпускании, если значение пересекло порог,
// возвращаем его к порогу
//------------------------------------------------------------------------------
void AxisHandler::doSpringReturn()
{
    if (springReturnLow >= 0.0f && value <= springReturnLow)
    {
        value = springReturnLow;
        sendControlSignal();
        return;
    }

    if (springReturnHigh >= 0.0f && value >= springReturnHigh)
    {
        value = springReturnHigh;
        sendControlSignal();
        return;
    }
}

//------------------------------------------------------------------------------
// Обработка клавиатурного ввода
//------------------------------------------------------------------------------
void AxisHandler::processKeyInput(const std::set<uint16_t> &pk)
{
    // Сброс в заданное положение
    if (resetKey != KEY_Undefined)
    {
        if (getKeyState(pk, resetKey) && isKeyModifier(pk, resetModkey))
        {
            value = resetValue;
            hold_direction = 0;
            sendControlSignal();
            return;
        }
    }

    if (getKeyState(pk, keyCodeInc) && isKeyModifier(pk, keyModIncName))
    {
        hold_direction = +1;
        return;
    }

    if (getKeyState(pk, keyCodeDec) && isKeyModifier(pk, keyModDecName))
    {
        hold_direction = -1;
        return;
    }

    // Клавиша отпущена — пружинный возврат
    doSpringReturn();
    hold_direction = 0;
}

//------------------------------------------------------------------------------
// Обработка мышиного ввода
//------------------------------------------------------------------------------
void AxisHandler::processMouseInput(uint32_t button, bool is_pressed)
{
    if (!is_pressed)
    {
        doSpringReturn();
        hold_direction = 0;
        return;
    }

    if (button == getButtonCode(incButtonName))
    {
        hold_direction = +1;
        return;
    }

    if (button == getButtonCode(decButtonName))
    {
        hold_direction = -1;
        return;
    }
}

//------------------------------------------------------------------------------
// Кадровый шаг: движение оси
//------------------------------------------------------------------------------
void AxisHandler::step(float t, float dt)
{
    (void) t;

    if (hold_direction == 0)
        return;

    float new_value = value + static_cast<float>(hold_direction) * speed * dt;
    new_value = std::clamp(new_value, minValue, maxValue);

    if (std::abs(new_value - value) > 1e-6f)
    {
        value = new_value;
        sendControlSignal();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString AxisHandler::getUsage() const
{
    return QString("ЛКМ — вверх | ПКМ — вниз");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString AxisHandler::getState() const
{
    return QString("%1").arg(static_cast<double>(getSignalValue()), 0, 'f', 3);
}
