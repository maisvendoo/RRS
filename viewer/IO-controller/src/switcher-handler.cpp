#include    <switcher-handler.h>

#include    <io-controller-keymap.h>
#include    <CfgReader.h>

#include    <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
SwitcherHandler::SwitcherHandler(QObject *parent) : ControlHandler(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool SwitcherHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    ControlHandler::load_config(cfg, secNode);

    QString keyInc;
    cfg.getString(secNode, "KeyNameInc", keyInc);
    keyCodeInc = KeySymbolsRRSMap.value(keyInc, KEY_Undefined);
    cfg.getString(secNode, "KeyModIncName", keyModIncName);

    QString keyDec;
    cfg.getString(secNode, "KeyNameDec", keyDec);
    keyCodeDec = KeySymbolsRRSMap.value(keyDec, KEY_Undefined);
    cfg.getString(secNode, "KeyModDecName", keyModDecName);

    int np = 2;
    cfg.getInt(secNode, "NumPositions", np);
    numPositions = static_cast<uint16_t>(std::max(np, 2));

    double tmp_min = 0.0, tmp_max = 1.0;
    cfg.getDouble(secNode, "MinValue", tmp_min);
    cfg.getDouble(secNode, "MaxValue", tmp_max);
    minValue = static_cast<float>(tmp_min);
    maxValue = static_cast<float>(tmp_max);

    int srl = -1, srh = -1;
    cfg.getInt(secNode, "SpringReturnLow", srl);
    cfg.getInt(secNode, "SpringReturnHigh", srh);
    springReturnLow = srl;
    springReturnHigh = srh;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int SwitcherHandler::currentIndex() const
{
    float range = maxValue - minValue;
    float step = range / static_cast<float>(numPositions - 1);
    return static_cast<int>(std::round((value - minValue) / step));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::sendNextPosition(int direction)
{
    int idx = currentIndex();
    int new_idx = std::clamp(idx + direction, 0,
                             static_cast<int>(numPositions) - 1);
    if (new_idx == idx) return;

    float range = maxValue - minValue;
    float step = range / static_cast<float>(numPositions - 1);
    value = minValue + static_cast<float>(new_idx) * step;
    sendControlSignal();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::processKeyInput(const std::set<uint16_t> &pk)
{
    if (getKeyState(pk, keyCodeInc) && isKeyModifier(pk, keyModIncName))
    {
        sendNextPosition(+1);
        hold_direction = +1;
        hold_time = 0.0f;
        spring_low_triggered = false;
        spring_high_triggered = false;
        return;
    }

    if (getKeyState(pk, keyCodeDec) && isKeyModifier(pk, keyModDecName))
    {
        sendNextPosition(-1);
        hold_direction = -1;
        hold_time = 0.0f;
        spring_low_triggered = false;
        spring_high_triggered = false;
        return;
    }

    hold_direction = 0;
    hold_time = 0.0f;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::processMouseInput(uint32_t button, bool is_pressed)
{
    if (!is_pressed)
    {
        hold_direction = 0;
        hold_time = 0.0f;
        return;
    }

    int dir = 0;
    if (button == CTRL_LEFT_MOUSE_BUTTON)  dir = +1;
    if (button == CTRL_RIGHT_MOUSE_BUTTON) dir = -1;
    if (dir == 0) return;

    sendNextPosition(dir);
    hold_direction = dir;
    hold_time = 0.0f;
    spring_low_triggered = false;
    spring_high_triggered = false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void SwitcherHandler::step(float t, float dt)
{
    (void) t;

    // Автоповтор при удержании клавиши
    if (hold_direction != 0)
    {
        hold_time += dt;
        if (hold_time >= HOLD_DELAY)
        {
            hold_time -= REPEAT_INTERVAL;
            sendNextPosition(hold_direction);
        }
        return;
    }

    // Пружинный возврат при отпускании клавиши
    int idx = currentIndex();

    if (springReturnLow >= 0 && idx == springReturnLow && !spring_low_triggered)
    {
        spring_low_triggered = true;
        float range = maxValue - minValue;
        float step = range / static_cast<float>(numPositions - 1);
        value = minValue + static_cast<float>(springReturnLow + 1) * step;
        sendControlSignal();
        return;
    }

    if (springReturnHigh >= 0 && idx == springReturnHigh && !spring_high_triggered)
    {
        spring_high_triggered = true;
        float range = maxValue - minValue;
        float step = range / static_cast<float>(numPositions - 1);
        value = minValue + static_cast<float>(springReturnHigh - 1) * step;
        sendControlSignal();
        return;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString SwitcherHandler::getUsage() const
{
    return QString("ЛКМ — вперёд | ПКМ — назад");
}