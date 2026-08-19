//------------------------------------------------------------------------------
//
//      Coupling interaction (интерактивная сцепка СА-3, рукава, краны)
//      ТЗ "Интерактивная сцепка realcoupling"
//
//      Машина состояний интерактивных операций по концам ПЕ:
//      - тормозной рукав: соединён / на кронштейне / болтается;
//      - концевой кран: открыт / закрыт;
//      - рычаг расцепного привода СА-3: натянут / отпущен.
//      Блокировки: двигаться с рукавом, не положенным на кронштейн, и
//      разъезд при соединённом рукаве - обрыв рукава и экстренное
//      торможение (разрыв ТМ); расцепка рычагом требует остановки.
//
//      Привязка к игроку в пешем режиме - ТЗ "Ходьба" (API готов).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_COUPLING_INTERACTION_H
#define     VEHICLE_COUPLING_INTERACTION_H

#include    <QString>

#include    <cstddef>

class VehicleDamageSystem;

//------------------------------------------------------------------------------
/// Интерактивная сцепка ПЕ (оба конца)
//------------------------------------------------------------------------------
class CouplingInteraction
{
public:

    /// Конец ПЕ
    enum End
    {
        Fwd = 0,
        Bwd = 1
    };

    /// Состояние тормозного рукава (ТЗ, п.3-6)
    enum class HoseState
    {
        Connected = 0,  ///< Соединён с соседом
        OnHolder = 1,   ///< Подвешен на кронштейн
        Hanging = 2     ///< Болтается (нельзя двигаться!)
    };

    CouplingInteraction() = default;

    /// Загрузка секции [CouplingInteraction]
    void loadConfig(QString cfg_path);

    /// Соединить рукав своего конца с соседом (блокировка: кран закрыть
    /// нельзя при соединении, сначала краны, потом рукав - как в жизни
    /// допускается любое, но соединение при открытом кране даёт толчок)
    bool connectHose(End end);

    /// Повесить рукав на кронштейн (только отсоединённый)
    bool hangHose(End end);

    /// Открыть/закрыть концевой кран
    void setEndValve(End end, bool open);

    /// Натянуть рычаг расцепного привода (требует скорости < 0.5 м/с)
    bool pullUncoupleLever(End end, double speed);

    /// Шаг: проверка обрыва рукава при движении (рукав болтается или
    /// разъезд с соединённым рукавом)
    void step(double dt, double velocity, VehicleDamageSystem& damage);

    HoseState getHoseState(End end) const;
    bool isEndValveOpen(End end) const;
    bool isLeverPulled(End end) const;

    /// Рукав оборван (авария)
    bool isHoseTorn(End end) const;

    QString getDebugMsg() const;

private:

    struct EndState
    {
        HoseState hose = HoseState::OnHolder;
        bool valve_open = false;
        bool lever_pulled = false;
        bool hose_torn = false;
    };

    EndState ends[2];

    /// Скорость, выше которой болтающийся рукав обрывается, м/с
    double tear_speed = 1.0;
};

#endif // VEHICLE_COUPLING_INTERACTION_H
