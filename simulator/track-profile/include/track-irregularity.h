//------------------------------------------------------------------------------
//
//      Track irregularities profile for RRS
//      Irregularity types and geometry shapes
//
//      Все координаты - метры вдоль траектории (пути), все амплитуды -
//      метры по вертикали (положительная - выпучивание вверх).
//      Неровность является свойством геометрии пути: один и тот же
//      участок даёт одинаковый профиль любому поезду (ТЗ "Неровности пути").
//
//------------------------------------------------------------------------------

#ifndef     TRACK_IRREGULARITY_H
#define     TRACK_IRREGULARITY_H

#include    "track-profile-export.h"

#include    <cstdint>

namespace track
{

/// Рельс, к которому относится неровность
enum class RailSide : std::uint8_t
{
    Left = 0,   ///< Левый рельс по ходу возрастания координаты пути
    Right = 1,  ///< Правый рельс
    Both = 2    ///< Оба рельса (одинаковая неровность)
};

/// Тип неровности
enum class IrregularityType : std::uint8_t
{
    RailJoint = 0,  ///< Рельсовый стык: короткая резкая просадка
    Weld = 1,       ///< Сварной шов: плавная, зависит от качества сварки
    Depression = 2, ///< Просадка пути: протяжённая с плавным входом/выходом
    LocalDip = 3,   ///< Локальная выбоина: короткая плавная просадка
    Bulge = 4,      ///< Выпучивание: локальное возвышение
    WaveWear = 5,   ///< Волнообразный износ: синусоида на протяжении участка
    Switch = 6      ///< Стрелочный перевод (крестовина): резкий двойной
                    ///< импульс на входе/выходе зоны + жёсткий удар в центре
};

//------------------------------------------------------------------------------
/// Геометрическая неровность пути.
///
/// Формы (все непрерывны по первой производной - без бесконечного рывка):
/// - RailJoint: половина волны косинуса, длина 0.08-0.3 м, резкая;
/// - Weld: сглаженный синус (sin^2), амплитуда снижается качеством сварки;
/// - Depression: трапеция со сглаженными склонами (smoothstep),
///   длины склонов EntryLength/ExitLength задаются отдельно;
/// - LocalDip/Bulge: sin^2-горка (вниз/вверх);
/// - WaveWear: синус с окном sin^2 на протяжении Length,
///   длина волны задается Wavelength;
/// - Switch: зона перевода длины Length: импульсы формы стыка x1.5
///   на входе и выходе зоны (стыки усовиков/контррельсов) + жёсткий
///   удар двойной амплитуды в центре (крестовина), ТЗ "Неровности", п.15.
//------------------------------------------------------------------------------
struct TRACKPROFILE_EXPORT Irregularity
{
    IrregularityType type = IrregularityType::LocalDip;

    /// Координата центра (для Depression - центр участка), м
    double coord = 0.0;
    /// Амплитуда: максимальное отклонение, м (для Bulge - положительная)
    double amplitude = 0.003;
    /// Полная длина зоны воздействия, м
    double length = 0.15;
    /// Depression: длина плавного входа, м
    double entry_length = 3.0;
    /// Depression: длина плавного выхода, м
    double exit_length = 3.0;
    /// WaveWear: длина волны износа, м
    double wavelength = 0.4;
    /// Weld: качество сварки 0..1 (1 - идеальный шов, не ощущается)
    double quality = 0.7;
    /// RailJoint: степень износа стыка 0..1 (масштабирует амплитуду)
    double wear = 0.3;

    RailSide side = RailSide::Both;

    /// Вертикальное смещение рельса в точке path_coord, м.
    /// Вне зоны воздействия - 0. Функция чистая (без состояния)
    double offset(double path_coord) const;

    /// Координата начала зоны воздействия, м
    double beginCoord() const;

    /// Координата конца зоны воздействия, м
    double endCoord() const;

    /// Разбор типа по имени ("joint", "weld", "depression", "dip",
    /// "bulge", "wave", "switch"). При ошибке - LocalDip и ok = false
    static IrregularityType typeFromString(const char* name, bool* ok = nullptr);

    /// Имя типа
    static const char* typeToString(IrregularityType type);

    /// Разбор стороны по имени ("left", "right", "both")
    static RailSide sideFromString(const char* name, bool* ok = nullptr);
};

} // namespace track

#endif // TRACK_IRREGULARITY_H
