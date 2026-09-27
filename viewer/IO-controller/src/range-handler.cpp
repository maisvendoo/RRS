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

    // Позиция по горячим клавишам
    if (!positionModkey.isEmpty() && !posKeys.empty())
    {
        for (int i = 0; i < posKeys.size(); ++i)
        {
            if (getKeyState(pressed_keys, posKeys[i].hotKey) && isKeyModifier(pressed_keys, positionModkey))
            {
                value = posKeys[i].value;
                sendControlSignal();
                break;
            }
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

    cfg.getString(secNode, "PositionModkey", positionModkey);
    QString tmp = "";
    cfg.getString(secNode, "PositionKeys", tmp);
    QStringList pos_keys = tmp.split(',');

    for (auto &pos_key : pos_keys)
    {
        pos_key.remove(' ');
        pos_key.remove('(');
        pos_key.remove(')');

        auto tokens = pos_key.split(';');

        if (tokens.size() < 2)
        {
            continue;
        }

        position_t pos;
        pos.hotKey = KeySymbolsRRSMap.value(tokens[0], KEY_Undefined);
        bool ok = false;
        pos.value = tokens[1].toFloat(&ok);

        if (!ok)
        {
            continue;
        }

        posKeys.push_back(pos);
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString RangeHandler::getUsage() const
{
    QString inc;
    QString dec;

    switch (getButtonCode(incButtonName))
    {
    case CTRL_LEFT_MOUSE_BUTTON: inc = "ЛКМ"; break;
    case CTRL_RIGHT_MOUSE_BUTTON: inc = "ПКМ"; break;
    case CTRL_MIDDLE_MOUSE_BUTTON: inc = "СКМ"; break;
    default:

        inc = "Не назначено";
        break;
    }

    switch (getButtonCode(decButtonName))
    {
    case CTRL_LEFT_MOUSE_BUTTON: dec = "ЛКМ"; break;
    case CTRL_RIGHT_MOUSE_BUTTON: dec = "ПКМ"; break;
    case CTRL_MIDDLE_MOUSE_BUTTON: dec = "СКМ"; break;
    default:

        dec = "Не назначено";
        break;
    }

    return QString("Позиция: %1 — увеличение | %2 — уменьшение").arg(inc).arg(dec);
}
