#include    <switcher-handler.h>

#include    <io-controller-keymap.h>
#include    <CfgReader.h>

#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitcherHandler::SwitcherHandler(QObject *parent) : RangeHandler(parent)
{

}

//------------------------------------------------------------------------------
// Загрузка конфигурации многопозиционного переключателя
//------------------------------------------------------------------------------
bool SwitcherHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    RangeHandler::load_config(cfg, secNode);

    // Количество фиксированных позиций (от 2)
    int np = 2;
    cfg.getInt(secNode, "NumPositions", np);
    numPositions = static_cast<uint16_t>(std::max(np, 2));

    // Минимальное и максимальное значение сигнала (0..1 по умолчанию)
    double tmp_min = 0.0, tmp_max = 1.0;
    cfg.getDouble(secNode, "MinValue", tmp_min);
    cfg.getDouble(secNode, "MaxValue", tmp_max);
    minValue = static_cast<float>(tmp_min);
    maxValue = static_cast<float>(tmp_max);

    range = maxValue - minValue;

    if (numPositions >= 2)
    {
        val_step = range / static_cast<float>(numPositions - 1);
    }
    else
    {
        val_step = 0.0;
    }

    // Пружинный возврат из крайних положений (опционально)
    int srl = -1, srh = -1;
    cfg.getInt(secNode, "SpringReturnLow", srl);   // возврат с нижней позиции (+1)
    cfg.getInt(secNode, "SpringReturnHigh", srh);  // возврат с верхней позиции (-1)
    springReturnLow = srl;
    springReturnHigh = srh;

    QString tmp = "";
    cfg.getString(secNode, "PositionsNames", tmp);
    positionNames = tmp.split(',');            

    return true;
}

//------------------------------------------------------------------------------
// Текущий индекс позиции (0..numPositions-1) на основе собственного value
//------------------------------------------------------------------------------
int SwitcherHandler::currentIndex() const
{
    return static_cast<int>(qAbs(std::round((value - minValue) / val_step)));
}

//------------------------------------------------------------------------------
// Отправить команду на шаг в заданном направлении
//------------------------------------------------------------------------------
void SwitcherHandler::sendNextPosition(int direction)
{
    int idx = currentIndex();
    int new_idx = std::clamp(idx + direction, 0,
                             static_cast<int>(numPositions) - 1);
    if (new_idx == idx) return;  // уже в крайнем положении

    float new_value = minValue + static_cast<float>(new_idx) * val_step;
    setValueIfChanged(new_value);
}

//------------------------------------------------------------------------------
// Обработка клавиатурного ввода
//------------------------------------------------------------------------------
void SwitcherHandler::processKeyInput(const std::set<uint16_t> &pk)
{
    RangeHandler::processKeyInput(pk);

    // Клавиша увеличения нажата?
    if (getKeyState(pk, keyCodeInc) && isKeyModifier(pk, keyModIncName))
    {
        sendNextPosition(+1);
        hold_direction = +1;           // запоминаем направление для автоповтора
        hold_time = 0.0f;
        spring_low_triggered = false;  // сбрасываем флаги возврата
        spring_high_triggered = false;
        return;
    }

    // Клавиша уменьшения нажата?
    if (getKeyState(pk, keyCodeDec) && isKeyModifier(pk, keyModDecName))
    {
        sendNextPosition(-1);
        hold_direction = -1;
        hold_time = 0.0f;
        spring_low_triggered = false;
        spring_high_triggered = false;
        return;
    }        

    // Ни одна клавиша не нажата — сбрасываем удержание
    hold_direction = 0;
    hold_time = 0.0f;
}

//------------------------------------------------------------------------------
// Пружинный возврат из крайних положений: при отпускании на граничной
// позиции автоматически делается шаг внутрь (нижняя → +1, верхняя → -1)
//------------------------------------------------------------------------------
void SwitcherHandler::doSpringReturn()
{
    if (springReturnLow < 0 && springReturnHigh < 0) return;

    int idx = currentIndex();

    // Нижняя крайняя позиция — шагнуть вверх
    if (springReturnLow >= 0 && idx == springReturnLow && !spring_low_triggered)
    {
        spring_low_triggered = true;        
        value = minValue + static_cast<float>(springReturnLow + 1) * val_step;
        sendControlSignal();
        return;
    }

    // Верхняя крайняя позиция — шагнуть вниз
    if (springReturnHigh >= 0 && idx == springReturnHigh && !spring_high_triggered)
    {
        spring_high_triggered = true;        
        value = minValue + static_cast<float>(springReturnHigh - 1) * val_step;
        sendControlSignal();
        return;
    }
}

//------------------------------------------------------------------------------
// Обработка мышиного ввода
//------------------------------------------------------------------------------
void SwitcherHandler::processMouseInput(uint32_t button, bool is_pressed)
{
    if (!feedback_signals) return;

    // Отпускание кнопки — сброс удержания и проверка пружинного возврата
    if (!is_pressed)
    {
        doSpringReturn();
        hold_direction = 0;
        hold_time = 0.0f;
        spring_low_triggered = false;
        spring_high_triggered = false;
        return;
    }

    // Нажатие — шаг в соответствующую сторону
    int dir = 0;
    if (button == getButtonCode(incButtonName))  dir = +1;
    if (button == getButtonCode(decButtonName)) dir = -1;
    if (dir == 0) return;

    sendNextPosition(dir);
    hold_direction = dir;
    hold_time = 0.0f;
    spring_low_triggered = false;
    spring_high_triggered = false;
}

//------------------------------------------------------------------------------
// Кадровый шаг: автоповтор при удержании + страховочный spring return
//------------------------------------------------------------------------------
void SwitcherHandler::step(float t, float dt)
{
    (void) t;

    // Автоповтор: пока клавиша/кнопка зажата, с заданным интервалом
    if (hold_direction != 0)
    {
        hold_time += dt;
        if (hold_time >= HOLD_DELAY)
        {
            hold_time -= REPEAT_INTERVAL;
            sendNextPosition(hold_direction);
        }
        return;  // не проверяем spring return, пока зажато
    }

    // Пружинный возврат: при достижении крайней позиции (страховка,
    // если вызов из processKeyInput/processMouseInput не сработал)
    if (springReturnLow >= 0)
    {
        int idx = currentIndex();
        if (idx == springReturnLow && !spring_low_triggered)
        {
            spring_low_triggered = true;
            value = minValue + static_cast<float>(springReturnLow + 1) * (maxValue - minValue) / (numPositions - 1);
            sendControlSignal();
            return;
        }
    }

    if (springReturnHigh >= 0)
    {
        int idx = currentIndex();
        if (idx == springReturnHigh && !spring_high_triggered)
        {
            spring_high_triggered = true;
            value = minValue + static_cast<float>(springReturnHigh - 1) * (maxValue - minValue) / (numPositions - 1);
            sendControlSignal();
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString SwitcherHandler::getState() const
{
    int idx = currentIndex();

    if (positionNames.empty() || idx >= positionNames.size())
    {
        return QString("%1").arg(idx, 2);
    }

    return positionNames[idx];
}
