#include "CabElements.h"

#include "CfgReader.h"

#include <map>

//------------------------------------------------------------------------------
// Клавиша конфига -> keysym (пространство KEY_* / vsg::KeySymbol общее)
//------------------------------------------------------------------------------
static std::uint16_t keyFromString(const QString& name)
{
    // Буквы - в нижнем регистре keysym (конвенция X11/vsg и серверного
    // key-symbols.h: KEY_U = 'u'), иначе тумблеры на буквенных клавишах
    // сервер не сопоставит со своими TriggerControl
    static const std::map<QString, std::uint16_t> keys = {
        {"1", 0x31}, {"2", 0x32}, {"3", 0x33}, {"4", 0x34},
        {"5", 0x35}, {"6", 0x36}, {"7", 0x37},
        {"I", 0x69}, {"O", 0x6F}, {"U", 0x75}, {"P", 0x70},
        {"Y", 0x79}, {"T", 0x74}, {"K", 0x6B}, {"L", 0x6C},
        {"V", 0x76}, {"G", 0x67}, {"J", 0x6A}, {"H", 0x68},
        {"Z", 0x7A}, {"C", 0x63}, {"X", 0x78},
        {"D", 0x64}, {"A", 0x61}, {"W", 0x77}, {"S", 0x73},
        {"Quote", 0x27}, {"Semicolon", 0x3B},
        {"Leftbracket", 0x5B}, {"Rightbracket", 0x5D},
        {"Tilde", 0x7E}, {"BackSpace", 0xFF08},
    };

    auto it = keys.find(name);
    return (it != keys.end()) ? it->second : 0;
}

//------------------------------------------------------------------------------
// Модификатор конфига -> keysym левой клавиши-модификатора
//------------------------------------------------------------------------------
static std::uint16_t modifierFromString(const QString& name)
{
    if (name == "shift") return 0xFFE1;  // KEY_Shift_L
    if (name == "ctrl")  return 0xFFE3;  // KEY_Control_L
    if (name == "alt")   return 0xFFE9;  // KEY_Alt_L
    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<CabElement> loadCabElements(const std::string& cfg_path)
{
    std::vector<CabElement> elements;

    CfgReader cfg;
    if (!cfg.load(cfg_path.c_str()))
    {
        return elements;
    }

    QDomNode section = cfg.getFirstSection("CabElement");

    while (!section.isNull())
    {
        CabElement elem;

        QString tmp = "";

        if (cfg.getString(section, "Name", tmp))
            elem.name = tmp.toStdString();

        tmp = "";
        if (cfg.getString(section, "Hint", tmp))
            elem.hint = tmp.toStdString();

        tmp = "";
        if (!cfg.getString(section, "ModelName", tmp) || tmp.isEmpty())
        {
            section = cfg.getNextSection();
            continue;
        }
        elem.model_name = tmp.toStdString();

        tmp = "";
        if (cfg.getString(section, "Type", tmp))
            elem.type = tmp.toStdString();

        int ivalue = -1;
        if (cfg.getInt(section, "SignalID", ivalue))
            elem.signal_id = ivalue;

        tmp = "";
        if (cfg.getString(section, "KeyOn", tmp))
        {
            elem.key_on = keyFromString(tmp);
            tmp = "";
            if (cfg.getString(section, "ModOn", tmp))
                elem.mod_on = modifierFromString(tmp);
        }

        tmp = "";
        if (cfg.getString(section, "KeyOff", tmp))
        {
            elem.key_off = keyFromString(tmp);
            tmp = "";
            if (cfg.getString(section, "ModOff", tmp))
                elem.mod_off = modifierFromString(tmp);
        }

        tmp = "";
        if (cfg.getString(section, "StateNames", tmp))
            elem.state_names = tmp.toStdString();

        ivalue = 0;
        cfg.getInt(section, "StatePositions", ivalue);
        elem.state_positions = ivalue;

        ivalue = -1;
        if (!cfg.getInt(section, "SignalID2", ivalue))
        {
            ivalue = -1;
        }
        elem.signal_id2 = ivalue;

        tmp = "";
        if (cfg.getString(section, "StateMode", tmp))
            elem.state_mode = tmp.toStdString();

        elem.interactable = (elem.key_on != 0) || (elem.key_off != 0);

        elements.push_back(std::move(elem));

        section = cfg.getNextSection();
    }

    return elements;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CabTooltipState& cabTooltip()
{
    static CabTooltipState state;
    return state;
}
