//------------------------------------------------------------------------------
//
//      Wheel wear system (износ бандажей колёсных пар)
//      ТЗ "43-47", раздел 1: износ бандажей и рельсов
//
//      Износ накапливается от реальной эксплуатации, а не от времени:
//      пробег и тоннаж, энергия проскальзывания (боксование/юз), работа
//      тормозных колодок, кривизна пути и состояние поверхности рельса.
//      Дёшево накапливается каждый кадр (аккумуляторы), пересчёт износа
//      выполняется на wear-тике 5 Гц (ТЗ "Оптимизация", п.16).
//
//      Связь с остальной физикой: износ меняет эффективную коничность
//      профиля катания - conicity = base + wear * gain - и передаётся в
//      поперечную динамику (setConicity): изношенный профиль снижает
//      критическую скорость виляния (звено "износ -> профиль -> динамика",
//      которое раньше было разорвано).
//
//      Обточка (reprofile): профиль восстанавливается, счётчик обточек и
//      уменьшение диаметра колеса учитываются API getWheelRadiusFactor()
//      (мутация rk остаётся за потребителем - модель колёс читает
//      радиусы из конфига ПЕ).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_WHEEL_WEAR_H
#define     VEHICLE_WHEEL_WEAR_H

#include    <QString>

#include    <cstddef>
#include <vector>

//------------------------------------------------------------------------------
/// Износ колёсных пар единицы ПС
//------------------------------------------------------------------------------
class WheelWearSystem
{
public:

    WheelWearSystem() = default;

    /// Загрузка секции [WheelWear]: num_axis - число осей,
    /// wheel_diameter_m - новый бандаж (для расчёта фактора радиуса)
    void loadConfig(QString cfg_path,
                    std::size_t num_axis,
                    double wheel_diameter_m);

    bool isEnabled() const;

    //--------- Аккумуляторы (вызов каждый кадр, дёшево) ---------

    /// Добавить энергию проскальзывания колеса о рельс, Дж (на ось)
    void addAxleSlipEnergy(std::size_t axle, double joules);

    /// Добавить работу тормозных колодок, Дж (на ось)
    void addAxleBrakeEnergy(std::size_t axle, double joules);

    //--------- Пересчёт износа (wear-тик 5 Гц) ---------

    /// dt - интервал wear-тика; velocity - скорость ПЕ; full_mass - масса
    /// с грузом; curvature - кривизна пути, 1/м; brake_shoe_max_temp -
    /// максимальная температура колодок (циклы нагрева); rail_condition -
    /// состояние поверхности рельса 0..1 (0 - чистый, 1 - забитый)
    void step(double dt,
              double velocity,
              double full_mass,
              double curvature,
              double brake_shoe_max_temp,
              double rail_condition);

    //--------- Датчики ---------

    /// Пробег с начала эксплуатации (сборки), км
    double getMileage() const;

    /// Тоннаж, т*км (полная масса на пройденный путь)
    double getTonnage() const;

    /// Глубина износа оси, мм
    double getAxleWearDepth(std::size_t axle) const;

    /// Средняя глубина износа, мм
    double getWearDepth() const;

    /// Нормализованный износ 0..1 (до предельной глубины обточки)
    double getWear() const;

    /// Эффективная коничность: base + wear * gain (для поперечной
    /// динамики - вызывается lateral_dynamics.setConicity)
    double getConicity() const;

    /// Базовая коничность нового бандажа (из [LateralDynamics])
    void setBaseConicity(double value);

    /// Осевые нагрузки в т*км (диагностика)
    double getAxleTonnage(std::size_t axle) const;

    //--------- Обточка (депо) ---------

    /// Обточка колёсных пар: профиль восстанавливается (износ в 0),
    /// диаметр уменьшается на минимальный съём + глубину износа.
    /// Ползуны сбрасывает вызывающий код (WheelFlatSystem::reset)
    void reprofile();

    /// Число выполненных обточек
    int getReprofileCount() const;

    /// Суммарное уменьшение диаметра оси от обточек, мм
    double getAxleDiameterReduction(std::size_t axle) const;

    /// Фактор радиуса оси: (D - 2*съём) / D относительно нового бандажа
    double getWheelRadiusFactor(std::size_t axle) const;

    QString getDebugMsg() const;

private:

    bool enabled = true;
    std::size_t num_axis = 4;

    /// Диаметр нового колеса, м
    double wheel_diameter = 0.95;

    /// Базисные накопители (общие на ПЕ)
    double mileage_km = 0.0;
    double tonnage_tkm = 0.0;       ///< т*км

    /// Износ на ось, мм
    std::vector<double> axle_wear_mm;

    /// Накопители энергии с последнего wear-тика, Дж (на ось)
    std::vector<double> axle_slip_energy;
    std::vector<double> axle_brake_energy;

    /// Тоннаж на ось, т*км (диагностика)
    std::vector<double> axle_tonnage;

    /// Циклы нагрева колодок выше порога (умножитель износа)
    int thermal_cycles = 0;
    bool was_hot = false;

    /// Обточки
    int reprofile_count = 0;
    std::vector<double> axle_reduction_mm;

    // Параметры износа (все - из конфига, ТЗ "43-47", п.11)

    /// Базовый износ катания, мм на т*км оси
    double wear_per_tkm = 1.7e-7;

    /// Износ от проскальзывания, мм/МДж (боксование/юз)
    double slip_wear_per_mj = 0.02;

    /// Износ от торможения, мм/МДж
    double brake_wear_per_mj = 0.005;

    /// Усиление износа в кривых: фактор = 1 + coeff * |curvature|
    double curvature_coeff = 500.0;

    /// Усиление износа от грязного рельса: фактор = 1 + coeff * condition
    double rail_condition_coeff = 0.5;

    /// Предельная глубина износа (нормировка), мм
    double wear_limit_mm = 3.0;

    /// Базовая коничность нового бандажа
    double base_conicity = 0.05;

    /// Прирост коничности при полном износе
    double conicity_gain = 0.15;

    /// Порог температуры колодок для цикла, град. C
    double thermal_cycle_temp = 250.0;

    /// Минимальный съём на обточку, мм
    double reprofile_cut_mm = 1.0;

    /// Дополнительный съём на каждый мм износа, мм/мм
    double reprofile_cut_per_wear = 2.0;
};

#endif // VEHICLE_WHEEL_WEAR_H
