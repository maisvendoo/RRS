#ifndef EDITOR_MASK_H
#define EDITOR_MASK_H

namespace editor2
{

/// Битовые маски для разделения проходов рендера и пересечений
enum Mask : unsigned int
{
    /// Основная сцена (рендерится первой)
    MASK_SCENE = 0x01,
    /// Рисуется после MASK_SCENE
    MASK_GUI1 = 0x02,
    /// Рисуется после MASK_GUI1
    MASK_GUI2 = 0x04,
    /// Реагирует на пересечения (клики мышью)
    MASK_CLICKABLE = 0x08
};

} // namespace editor2

#endif // EDITOR_MASK_H
