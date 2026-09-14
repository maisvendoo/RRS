#include "editor/KeyBindings.h"

#include <CfgReader.h>

#include <cctype>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

/// Имя поля действия в секции Keys конфига
static const char* get_setting_name(Action action)
{
    switch (action)
    {
        case ACTION_DELETE_OBJECTS:
        {
            return "DeleteObjects";
        }
        case ACTION_FOCUS_ON_SELECTION:
        {
            return "FocusOnSelection";
        }
        case ACTION_COPY_OBJECTS:
        {
            return "CopyObjects";
        }
        case ACTION_PASTE_OBJECTS:
        {
            return "PasteObjects";
        }
        case ACTION_HIDE_OBJECTS:
        {
            return "HideObjects";
        }
        case ACTION_GIZMO_TRANSLATE:
        {
            return "GizmoTranslate";
        }
        case ACTION_GIZMO_ROTATE:
        {
            return "GizmoRotate";
        }
        case ACTION_GIZMO_SCALE:
        {
            return "GizmoScale";
        }
        case ACTION_SAVE_ROUTE:
        {
            return "SaveRoute";
        }
        case ACTION_UNDO:
        {
            return "UndoCommand";
        }
        case ACTION_REDO:
        {
            return "RedoCommand";
        }
        default:
        {
            return "Unknown";
        }
    }
}

/// Клавиша в виде строки ("Ctrl + Z" -> "Z", Delete -> "Delete")
static std::string key_to_string(vsg::KeySymbol key)
{
    switch (key)
    {
        case vsg::KEY_Delete:
        {
            return "Delete";
        }
        case vsg::KEY_Escape:
        {
            return "Escape";
        }
        default:
        {
            break;
        }
    }

    const int code = static_cast<int>(key);

    // Печатаемый ASCII-символ
    if (code >= 0x21 && code <= 0x7e)
    {
        return std::string(1,
            static_cast<char>(std::toupper(static_cast<unsigned char>(code))));
    }

    return std::to_string(code);
}

/// Токен ("z", "delete") -> клавиша
static vsg::KeySymbol parse_key(const std::string& token)
{
    if (token == "delete")
    {
        return vsg::KEY_Delete;
    }

    if (token == "escape" || token == "esc")
    {
        return vsg::KEY_Escape;
    }

    if (token.empty())
    {
        return static_cast<vsg::KeySymbol>(0);
    }

    return static_cast<vsg::KeySymbol>(token.front());
}

const char* to_c_string(Action action)
{
    switch (action)
    {
        case ACTION_DELETE_OBJECTS:
        {
            return "Удалить объекты";
        }
        case ACTION_FOCUS_ON_SELECTION:
        {
            return "Фокус на выделении";
        }
        case ACTION_COPY_OBJECTS:
        {
            return "Копировать объекты";
        }
        case ACTION_PASTE_OBJECTS:
        {
            return "Вставить объекты";
        }
        case ACTION_HIDE_OBJECTS:
        {
            return "Скрыть/показать объекты";
        }
        case ACTION_GIZMO_TRANSLATE:
        {
            return "Гизмо: цикл (перемещение)";
        }
        case ACTION_GIZMO_ROTATE:
        {
            return "Гизмо: поворот";
        }
        case ACTION_GIZMO_SCALE:
        {
            return "Гизмо: масштаб";
        }
        case ACTION_SAVE_ROUTE:
        {
            return "Сохранить маршрут";
        }
        case ACTION_UNDO:
        {
            return "Отменить (undo)";
        }
        case ACTION_REDO:
        {
            return "Повторить (redo)";
        }
        default:
        {
            return "Unknown";
        }
    }
}

KeyBindings::KeyBindings()
{
    // Дефолты = текущие клавиши редактора (до таблицы);
    // гизмо: G - цикл режимов, R/T - прямой выбор поворота/масштаба
    bindings_[ACTION_DELETE_OBJECTS] = KeyBinding{vsg::KEY_Delete, 0};
    bindings_[ACTION_FOCUS_ON_SELECTION] = KeyBinding{vsg::KEY_f, 0};
    bindings_[ACTION_COPY_OBJECTS] =
        KeyBinding{vsg::KEY_c, vsg::MODKEY_Control};
    bindings_[ACTION_PASTE_OBJECTS] =
        KeyBinding{vsg::KEY_v, vsg::MODKEY_Control};
    bindings_[ACTION_HIDE_OBJECTS] = KeyBinding{vsg::KEY_h, 0};
    bindings_[ACTION_GIZMO_TRANSLATE] = KeyBinding{vsg::KEY_g, 0};
    bindings_[ACTION_GIZMO_ROTATE] = KeyBinding{vsg::KEY_r, 0};
    bindings_[ACTION_GIZMO_SCALE] = KeyBinding{vsg::KEY_t, 0};
    bindings_[ACTION_SAVE_ROUTE] =
        KeyBinding{vsg::KEY_s, vsg::MODKEY_Control};
    bindings_[ACTION_UNDO] =
        KeyBinding{vsg::KEY_z, vsg::MODKEY_Control};
    bindings_[ACTION_REDO] =
        KeyBinding{vsg::KEY_y, vsg::MODKEY_Control};
}

void KeyBindings::read(CfgReader& cfg)
{
    const QString section = "Keys";

    static const std::map<std::string, std::uint16_t> modifier_names = {
        {"shift", vsg::MODKEY_Shift},
        {"ctrl", vsg::MODKEY_Control},
        {"control", vsg::MODKEY_Control},
        {"alt", vsg::MODKEY_Alt}
    };

    for (std::size_t i = 0; i < TOTAL_ACTIONS; ++i)
    {
        const Action action = static_cast<Action>(i);

        QString line;
        if (!cfg.getString(section, get_setting_name(action), line))
        {
            // Поля нет в конфиге - остаётся клавиша по умолчанию
            continue;
        }

        // Разбор строки вида "Ctrl + Shift + Z": последний токен -
        // клавиша, остальные - модификаторы
        const std::string lowered = line.toLower().toStdString();

        std::vector<std::string> tokens;
        std::string token;

        for (const char c : lowered)
        {
            if (c == '+' || c == ' ' || c == '\t')
            {
                if (!token.empty())
                {
                    tokens.push_back(token);
                    token.clear();
                }
            }
            else
            {
                token += static_cast<char>(
                    std::tolower(static_cast<unsigned char>(c)));
            }
        }

        if (!token.empty())
        {
            tokens.push_back(token);
        }

        if (tokens.empty())
        {
            continue;
        }

        KeyBinding binding;

        for (std::size_t t = 0; t + 1 < tokens.size(); ++t)
        {
            const auto found_it = modifier_names.find(tokens[t]);

            if (found_it != modifier_names.cend())
            {
                binding.modifiers = static_cast<std::uint16_t>(
                    binding.modifiers | found_it->second);
            }
        }

        binding.key = parse_key(tokens.back());

        if (binding.key != static_cast<vsg::KeySymbol>(0))
        {
            bindings_[i] = binding;
        }
    }
}

bool KeyBindings::save(const std::string& path) const
{
    std::ifstream input(path, std::ios::binary);

    if (!input.is_open())
    {
        return false;
    }

    const std::string content{std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};

    input.close();

    // Тело секции Keys с актуальными привязками
    std::string section = "    <Keys>\n";

    for (std::size_t i = 0; i < TOTAL_ACTIONS; ++i)
    {
        const Action action = static_cast<Action>(i);
        const std::string name = get_setting_name(action);

        section += "        <" + name + ">" + to_string(action) +
            "</" + name + ">\n";
    }

    section += "    </Keys>";

    std::string result;

    const std::size_t begin = content.find("<Keys>");

    if (begin != std::string::npos)
    {
        // Заменяем существующую секцию целиком
        const std::size_t end = content.find("</Keys>", begin);

        if (end == std::string::npos)
        {
            return false;
        }

        const std::size_t begin_tag = section.find("<Keys>");
        const std::string body = section.substr(begin_tag);

        result = content.substr(0, begin) + body +
            content.substr(end + std::string("</Keys>").size());
    }
    else
    {
        // Секции ещё нет - добавляем в конец контейнера Config
        const std::size_t config_end = content.find("</Config>");

        if (config_end == std::string::npos)
        {
            return false;
        }

        result = content.substr(0, config_end) + section + "\n" +
            content.substr(config_end);
    }

    std::ofstream output(path, std::ios::binary | std::ios::trunc);

    if (!output.is_open())
    {
        return false;
    }

    output << result;

    return output.good();
}

bool KeyBindings::matches(Action action, const vsg::KeyPressEvent& keyPress,
    bool ignore_shift) const
{
    const KeyBinding& binding =
        bindings_[static_cast<std::size_t>(action)];

    if (binding.key == static_cast<vsg::KeySymbol>(0))
    {
        return false;
    }

    constexpr std::uint16_t all_modifiers =
        vsg::MODKEY_Control | vsg::MODKEY_Shift | vsg::MODKEY_Alt;

    std::uint16_t pressed = static_cast<std::uint16_t>(
        keyPress.keyModifier & all_modifiers);

    std::uint16_t expected = binding.modifiers;

    if (ignore_shift)
    {
        pressed &= static_cast<std::uint16_t>(~vsg::MODKEY_Shift);
        expected &= static_cast<std::uint16_t>(~vsg::MODKEY_Shift);
    }

    if (pressed != expected)
    {
        return false;
    }

    return keyPress.keyBase == binding.key || keyPress.keyModified == binding.key;
}

void KeyBindings::assign(Action action, vsg::KeySymbol key,
    std::uint16_t modifiers)
{
    KeyBinding& binding = bindings_[static_cast<std::size_t>(action)];
    binding.key = key;
    binding.modifiers = modifiers;
}

const KeyBinding& KeyBindings::get(Action action) const
{
    return bindings_[static_cast<std::size_t>(action)];
}

std::string KeyBindings::to_string(Action action) const
{
    const KeyBinding& binding = bindings_[static_cast<std::size_t>(action)];

    std::string result;

    if (binding.modifiers & vsg::MODKEY_Control)
    {
        result += "Ctrl + ";
    }

    if (binding.modifiers & vsg::MODKEY_Shift)
    {
        result += "Shift + ";
    }

    if (binding.modifiers & vsg::MODKEY_Alt)
    {
        result += "Alt + ";
    }

    result += key_to_string(binding.key);

    return result;
}
