#ifndef EDITOR_KEY_BINDINGS_H
#define EDITOR_KEY_BINDINGS_H

#include <vsg/ui/KeyEvent.h>

#include <array>
#include <cstdint>
#include <string>

class CfgReader;

/// Действия редактора с переназначаемыми клавишами
enum Action
{
    ACTION_DELETE_OBJECTS,
    ACTION_FOCUS_ON_SELECTION,
    ACTION_COPY_OBJECTS,
    ACTION_PASTE_OBJECTS,
    ACTION_HIDE_OBJECTS,
    ACTION_GIZMO_TRANSLATE,
    ACTION_GIZMO_ROTATE,
    ACTION_GIZMO_SCALE,
    ACTION_SAVE_ROUTE,
    ACTION_UNDO,
    ACTION_REDO,
    TOTAL_ACTIONS
};

/// Отображаемое название действия (окно «Клавиши»)
const char* to_c_string(Action action);

/// Одна привязка: клавиша + модификаторы (Ctrl/Shift/Alt)
struct KeyBinding
{
    vsg::KeySymbol key = static_cast<vsg::KeySymbol>(0);
    std::uint16_t modifiers = 0;
};

/**
 * @brief Таблица переназначаемых клавиш редактора.
 *
 * Загружается из секции Keys конфига настроек редактора тем же
 * механизмом, что и остальные настройки (CfgReader), сохраняется
 * перезаписью секции Keys в этом же файле.
 *
 * Клавиши P и Esc (режим «Пути») и клавиши инструментов M, X, [ , ]
 * в таблицу не входят - это режимы, а не команды.
 */
class KeyBindings
{
public:
    /// Клавиши по умолчанию (текущие хоткеи редактора)
    KeyBindings();

    /// Прочитать секцию Keys конфига редактора
    void read(CfgReader& cfg);

    /// Перезаписать секцию Keys в XML-файле настроек редактора
    bool save(const std::string& path) const;

    /**
     * @brief Совпадает ли нажатая клавиша с привязкой действия.
     *
     * @param[in] action Действие.
     * @param[in] keyPress Событие нажатия клавиши.
     * @param[in] ignore_shift Игнорировать Shift (для Ctrl+Shift+Z -
     *                        redo, как в старом поведении).
     */
    bool matches(Action action, const vsg::KeyPressEvent& keyPress,
        bool ignore_shift = false) const;

    /// Назначить новую привязку
    void assign(Action action, vsg::KeySymbol key, std::uint16_t modifiers);

    const KeyBinding& get(Action action) const;

    /// Строка вида "Ctrl + Z" для отображения в GUI
    std::string to_string(Action action) const;

private:
    std::array<KeyBinding, TOTAL_ACTIONS> bindings_;
};

#endif // EDITOR_KEY_BINDINGS_H
