#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

/// Состояния конечного автомата редактора
enum class EditorState
{
    /// Маршрут не загружен — ждём выбора маршрута
    NO_ROUTE,
    /// Маршрут выбран — загрузить его в сцену в начале кадра
    LOAD_ROUTE,
    /// Маршрут загружен — режим редактирования
    EDIT_ROUTE
};

#endif // EDITOR_STATE_H
