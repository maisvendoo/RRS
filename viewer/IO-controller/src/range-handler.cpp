#include    <range-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
RangeHandler::RangeHandler(QObject *parent) : ControlHandler(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RangeHandler::processKeyInput(const std::set<uint16_t> &pressed_keys)
{
    // Сброс, если задана подобная настройка
    if (resetKey != KEY_Undefined)
    {
        if (getKeyState(pressed_keys, resetKey) && isKeyModifier(pressed_keys, resetModkey))
        {
            value = posForReset;
            sendControlSignal();
            return;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void RangeHandler::processMouseInput(uint32_t button, bool is_pressed)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool RangeHandler::load_config(CfgReader &cfg, QDomNode secNode)
{
    ControlHandler::load_config(cfg, secNode);

    QString resetKeyName = "";
    cfg.getString(secNode, "ResetKey", resetKeyName);
    resetKey = KeySymbolsRRSMap.value(resetKeyName, KEY_Undefined);

    cfg.getString(secNode, "ResetModkey", resetModkey);

    double reset_pos = 0.0;
    cfg.getDouble(secNode, "PositionForReset", reset_pos);
    posForReset = static_cast<float>(reset_pos);

    // Клавиша и модификатор для увеличения позиции
    QString keyInc;
    cfg.getString(secNode, "KeyNameInc", keyInc);
    keyCodeInc = KeySymbolsRRSMap.value(keyInc, KEY_Undefined);
    cfg.getString(secNode, "KeyModIncName", keyModIncName);

    // Клавиша и модификатор для уменьшения позиции
    QString keyDec;
    cfg.getString(secNode, "KeyNameDec", keyDec);
    keyCodeDec = KeySymbolsRRSMap.value(keyDec, KEY_Undefined);
    cfg.getString(secNode, "KeyModDecName", keyModDecName);

    cfg.getString(secNode, "IncButtonName", incButtonName);
    cfg.getString(secNode, "DecButtonName", decButtonName);

    return true;
}
