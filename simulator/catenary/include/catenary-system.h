//------------------------------------------------------------------------------
//
//      Catenary electrification system (КС, подстанции, секции)
//      ТЗ "Реалистичная контактная сеть + подстанции"
//
//      Электрическая инфраструктура маршрута по пикетажу: секции КС
//      (тип электрификации, фаза, питающая подстанция), тяговые
//      подстанции с реальным положением и пределом мощности,
//      нейтральные вставки. Напряжение на токоприёмнике - с провалом
//      от тока и расстояния до точки питания. Переходы 3 кВ <-> 25 кВ -
//      через нейтральные вставки.
//
//      Конфиг маршрута: catenary.conf (XML), пикетаж в метрах.
//
//------------------------------------------------------------------------------

#ifndef     CATENARY_SYSTEM_H
#define     CATENARY_SYSTEM_H

#include    "catenary-export.h"

#include    <QString>

#include    <cstddef>
#include <vector>

namespace catenary
{

/// Система электрификации
enum class SystemType
{
    DC3kV = 0,      ///< Постоянный ток 3 кВ
    AC25kV = 1,     ///< Переменный 25 кВ 50 Гц
    AC2x25kV = 2,   ///< 2x25 кВ (с УП)
    NotElectrified = 3
};

/// Фаза секции (для AC)
enum class Phase
{
    A = 0,
    B = 1,
    C = 2,
    None = 3
};

/// Тяговая подстанция
struct Substation
{
    QString id;
    double railway_coord = 0.0;     ///< Пикетаж, м
    double nominal_voltage = 25000.0;
    double max_power = 50.0e6;      ///< Вт
    bool switched_on = true;
    /// Приём рекуперации (обратная энергия в сеть/смежных потребителей)
    bool regen_reception = false;
    double max_regen_power = 0.0;   ///< Вт, приём подстанцией
};

/// Секция контактной сети
struct Section
{
    double begin = 0.0;             ///< Пикетаж начала, м
    double end = 0.0;               ///< Пикетаж конца, м
    SystemType type = SystemType::AC25kV;
    Phase phase = Phase::None;
    QString substation_id = "";
    bool switched_on = true;
};

/// Нейтральная вставка
struct NeutralInsert
{
    double begin = 0.0;
    double end = 0.0;
};

/// Состояние питания в точке
struct FeedState
{
    SystemType type = SystemType::NotElectrified;
    Phase phase = Phase::None;
    double voltage = 0.0;           ///< С провалом от тока, В
    bool powered = false;           ///< Есть питание
    bool in_neutral = false;        ///< Внутри нейтральной вставки
    bool accepts_regen = false;     ///< Сеть может принять рекуперацию
    double max_regen_power = 0.0;   ///< Предел приёма, Вт
    QString substation_id = "";
};

//------------------------------------------------------------------------------
/// Система электроснабжения маршрута
//------------------------------------------------------------------------------
class CATENARY_EXPORT CatenarySystem
{
public:

    /// Прочитать catenary.conf из каталога маршрута.
    /// Отсутствие файла - неэлектрифицированный маршрут
    void load(const QString& route_dir);

    /// Состояние питания в точке пикетажа при токе токоприёмника I, А
    FeedState getFeedState(double railway_coord, double current_a) const;

    /// Предел тока подстанции, питающей точку, А
    double getCurrentLimit(double railway_coord) const;

    /// Отключить/включить секцию (диспетчер, повреждение)
    void setSectionSwitched(double railway_coord, bool on);

private:

    const Section* findSection(double railway_coord) const;

    const Substation* findSubstation(const QString& id) const;

    const NeutralInsert* findNeutral(double railway_coord) const;

    std::vector<Substation> substations_;
    std::vector<Section> sections_;
    std::vector<NeutralInsert> neutrals_;

    /// Удельное сопротивление тяговой сети, Ом/км (усреднённо)
    double network_resistance = 0.12;
};

} // namespace catenary

#endif // CATENARY_SYSTEM_H
