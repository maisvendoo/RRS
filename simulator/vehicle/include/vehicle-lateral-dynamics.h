//------------------------------------------------------------------------------
//
//      Vehicle lateral dynamics (hunting, conicity, creep, flange contact)
//      ТЗ "Поперечная динамика подвижного состава"
//
//      Физическая модель (не синусоидальный генератор!):
//      - конический профиль бандажей: r_L = r0 + conicity*y, r_R = r0 -
//        conicity*y - разница радиусов качения создаёт кинематическое
//        возвращающее воздействие (самоцентрирование, кинематика Клингеля);
//      - крип-силы по теории Картера (линейные с насыщением трением
//        mu*Q, эллипс трения): продольный крип от разности радиусов и
//        рыскания, поперечный крип от боковой скорости;
//      - люфт до гребня + односторонний упор гребня (жёсткий контакт);
//      - поперечное подвешивание I ступени (колёсная пара - тележка) и
//        II ступени (тележка - кузов), боковые упоры с люфтом;
//      - возбуждение: боковые неровности плана линии (детерминированные),
//        кривизна пути (центробежная составляющая);
//      - интегрирование: полунеявный Эйлер, крип-члены неявные
//        (безусловно устойчивы при малой скорости);
//      - оценка критической скорости виляния - численно, по собственным
//        числам линеаризованной модели колёсной пары.
//
//      Ось Y - вправо от оси пути. y > 0 - смещение вправо.
//      Положительный Yaw - поворот по часовой стрелке сверху (влево нос).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_LATERAL_DYNAMICS_H
#define     VEHICLE_LATERAL_DYNAMICS_H

#include    <QString>

#include    <cstddef>
#include <cstdint>
#include    <functional>
#include    <vector>

//------------------------------------------------------------------------------
/// Поперечная динамика единицы подвижного состава
//------------------------------------------------------------------------------
class VehicleLateralDynamics
{
public:

    /// Состояние контакта колеса с рельсом (ТЗ "Динамика тележек", п.12)
    enum class ContactState : std::uint8_t
    {
        Rolling = 0,        ///< Чистое качение (крип ~0)
        Adhesion = 1,       ///< Сцепление с малым крипом
        PartialSlip = 2,    ///< Частичное проскальзывание
        Sliding = 3,        ///< Полное скольжение (юз/боксование)
        FlangeContact = 4,  ///< Контакт гребня с рельсом
        LossOfContact = 5   ///< Потеря контакта с рельсом
    };

    /// Источник бокового смещения оси пути: абсолютная координата пути -> м
    using LateralOffsetFn = std::function<double(double)>;

    VehicleLateralDynamics() = default;

    /// Загрузка параметров из секции [LateralDynamics] конфига ПЕ
    void loadConfig(QString cfg_path,
                    double full_mass,
                    std::size_t num_axis,
                    double length,
                    double wheel_diameter);

    bool isEnabled() const;
    bool isReady() const;

    /// Привязка источника боковых неровностей пути
    void setLateralOffsetSource(LateralOffsetFn fn);

    /// Шаг поперечной динамики.
    /// dt - шаг модели, с; velocity - скорость по координате пути, м/с;
    /// dir - ориентация ПЕ; train_coord - координата центра ПЕ;
    /// curvature - кривизна пути под ПЕ, 1/м; friction - коэфф. трения
    /// колеса о рельс; axle_load_factors - доли статической нагрузки
    /// на оси от вертикальной динамики (пусто - статика);
    /// cant_mm - возвышение наружного рельса, мм (Б16: "+" - левый рельс
    /// выше; участвует в эквивалентном боковом ускорении
    /// a_eq = v^2/R - g*cant/(2*b) вместо чистой центробежной)
    void step(double dt,
              double velocity,
              std::int8_t dir,
              double train_coord,
              double curvature,
              double friction,
              const std::vector<double>& axle_load_factors,
              double cant_mm = 0.0);

    //--------- Датчики ---------

    /// Боковое смещение кузова от оси пути, м
    double getBodyLateralPosition() const;

    /// Рыскание кузова, рад
    double getBodyYaw() const;

    /// Боковое ускорение кузова (фильтрованное), м/с^2
    double getBodyLateralAcceleration() const;

    /// Боковое смещение тележки, м
    double getBogieLateralPosition(std::size_t bogie) const;

    /// Рыскание тележки, рад
    double getBogieYaw(std::size_t bogie) const;

    /// Рыскание тележки относительно кузова, рад (п.2):
    /// BogieRelativeYaw = BogieYaw - VehicleYaw; в кривой растёт с 1/R
    double getBogieRelativeYaw(std::size_t bogie) const;

    /// Угол набегания колёсной пары, рад (п.5): угол между осью колёсной
    /// пары и касательной к пути в точке контакта. В кривой отличен от нуля
    /// (особенно у первой колёсной пары тележки)
    double getWheelsetAngleOfAttack(std::size_t axle) const;

    /// Состояние контакта колеса с рельсом
    ContactState getWheelContactState(std::size_t axle, int side) const;

    /// Задать текущую коничность бандажей (износ меняет профиль,
    /// п.29: износ -> коничность -> критическая скорость)
    void setConicity(double value);

    /// Текущая коничность бандажей (база системы износа колёс)
    double getConicity() const;

    /// Боковая ветровая нагрузка на кузов (ТЗ "43-47", п.2): сила, Н
    /// (положительная - вправо, +y) и высота приложения над уровнем
    /// рельсов, м. Сила смещает кузов, а через высоту приложения
    /// догружает/разгружает колёса тем же механизмом переноса
    /// нагрузки, что и центробежная в кривой
    void setWindLateralForce(double force_n, double height_m);

    /// Действующая боковая ветровая сила, Н (диагностика)
    double getWindLateralForce() const;

    /// Задать высоту центра масс над уровнем рельсов, м (перенос
    /// нагрузки на колёса в кривой), по умолчанию 1.8
    void setMassCenterHeight(double value);

    /// Боковое смещение колёсной пары, м
    double getWheelsetLateralPosition(std::size_t axle) const;

    /// Поперечная сила колеса о рельс (гребень + крип), Н.
    /// axle - ось, side: 0 - левое, 1 - правое
    double getWheelLateralForce(std::size_t axle, int side) const;

    /// Отношение Y/Q для колеса (критерий Надаля сравнивается с
    /// пределом tan(flange_angle - atan(mu)))
    double getWheelYQ(std::size_t axle, int side) const;

    /// Нарушение критерия безопасности схода (Надаль) хотя бы одним колесом
    bool isDerailmentCriterionViolated() const;

    /// Оценка критической скорости виляния, м/с (по линеаризованной
    /// модели колёсной пары в тележке, вычисляется при загрузке)
    double getCriticalSpeed() const;

    /// Амплитуда виляния кузова (RMS за ~5 с), м
    void resetMetrics();
    double getLateralRMS() const;

    /// Недовозвышение (cant deficiency), м/с^2: остаточное поперечное
    /// ускорение a_eq = v^2/R - g*cant/(2*b) с учётом возвышения.
    /// Датчик для систем комфорта и контроля вписывания в кривую
    double getCantDeficiency() const;

    /// Строка диагностики
    QString getDebugMsg() const;

private:

    struct AxleState
    {
        double y = 0.0;     ///< Боковое смещение, м
        double dy = 0.0;    ///< Боковая скорость, м/с
        double psi = 0.0;   ///< Рыскание, рад
        double dpsi = 0.0;  ///< Скорость рыскания, рад/с
    };

    struct BogieState
    {
        double y = 0.0;
        double dy = 0.0;
        double psi = 0.0;
        double dpsi = 0.0;
    };

    void integrateSubstep(double h,
                          double velocity,
                          std::int8_t dir,
                          double train_coord,
                          double curvature,
                          double friction,
                          double cant_mm);

    /// Численная оценка критической скорости: максимальная скорость,
    /// при которой вещественная часть собственного числа линеаризованной
    /// модели колёсной пары (y, psi) с вязким демпфированием подвески
    /// остаётся отрицательной
    double estimateCriticalSpeed() const;

    bool enabled = false;
    bool ready = false;

    LateralOffsetFn lateral_offset;

    // Геометрия
    std::size_t num_axis = 4;
    std::size_t num_bogies = 2;
    double length = 14.0;
    double wheel_radius = 0.475;
    std::vector<double> axle_offset;        ///< Смещение оси от центра, м
    std::vector<std::size_t> axle_bogie;
    std::vector<double> bogie_offset;

    // Профиль колеса и путь
    double conicity = 0.05;         ///< Коничность бандажа (tan угла конуса)
    double half_gauge = 0.7465;     ///< Полурасстояние точек контакта, м
    double flange_clearance = 0.023;///< Люфт до гребня, м
    double flange_stiffness = 5.0e7;///< Жёсткость контакта гребня, Н/м
    double flange_damping = 2.0e5;  ///< Демпфирование гребня, Н*с/м
    double flange_angle = 65.0;     ///< Угол наклона гребня, град (Надаль)

    // Массы, кг
    double full_mass = 25000.0;
    double wheelset_mass = 1500.0;
    double bogie_mass = 2200.0;
    double body_mass = 18000.0;

    /// Высота центра масс над уровнем рельсов, м (перенос нагрузки
    /// на колёса в кривой: доля = a*h_cm/(half_gauge*g))
    double mass_center_height = 1.8;

    /// Боковая ветровая нагрузка на кузов, Н (+ вправо) и высота
    /// приложения над уровнем рельсов, м
    double wind_lateral_force = 0.0;
    double wind_force_height = 1.8;

    // Инерция рыскания, кг*м^2
    double wheelset_yaw_inertia = 900.0;
    double bogie_yaw_inertia = 2000.0;
    double body_yaw_inertia = 300000.0;

    // Крип (Калкер, линейные коэффициенты, на колёсную пару)
    double creep_longitudinal = 9.0e6;  ///< f11, Н
    double creep_lateral = 8.0e6;       ///< f22, Н

    /// Спиновый коэффициент крипа f33·a·γ, Н*м*с: спин колеса в кривой
    /// (вращение контактного пятна, ω_spin = v/R) создаёт момент
    /// M_spin = c_spin·(v/R) на рыскание колёсной пары (ТЗ п.20)
    double spin_creep_coeff = 3400.0;

    // Подвешивание (на колёсную пару / на тележку)
    double primary_lateral_stiffness = 6.0e6;   ///< Н/м
    double primary_lateral_damping = 3.0e4;     ///< Н*с/м
    double primary_yaw_stiffness = 3.0e6;       ///< Н*м/рад (на пару пружин)
    double primary_yaw_free_play = 0.0;         ///< Люфт рыскания колёсной пары в тележке, рад
    double bogie_yaw_stiffness = 2.0e5;         ///< Возвратный момент тележки, Н*м/рад
    double bogie_yaw_free_play = 0.0;           ///< Люфт поворота тележки, рад
    double secondary_lateral_stiffness = 3.0e5; ///< Н/м на тележку
    double secondary_lateral_damping = 3.0e4;
    double side_bearer_clearance = 0.06;        ///< Люфт боковых упоров, м
    double side_bearer_stiffness = 5.0e6;       ///< Н/м

    double substep = 0.001;

    // Состояния
    std::vector<AxleState> axles;
    std::vector<BogieState> bogies;
    AxleState body;

    // Датчики
    double body_lateral_accel = 0.0;
    std::vector<double> wheel_yq;            ///< Y/Q на колесо: ось x сторона
    std::vector<double> wheel_lateral_force; ///< Y на колесо, Н
    std::vector<double> wheel_creep_ratio;   ///< Насыщение крипа 0..1+: ось x сторона
    std::vector<double> wheel_angle_of_attack;///< Угол набегания оси, рад
    std::vector<std::uint8_t> wheel_contact_state; ///< ContactState: ось x сторона
    std::vector<double> axle_rel_y;          ///< Смещение оси отн. оси пути, м
    bool derailment_criterion = false;
    double critical_speed = 100.0 / 3.6;

    /// Недовозвышение последнего шага, м/с^2 (датчик)
    double cant_deficiency = 0.0;

    // RMS боковых колебаний кузова (окно ~5 с, прореживание 10 мс)
    double lateral_sq_sum = 0.0;
    std::vector<double> lateral_window;
    double metrics_timer = 0.0;
};

#endif // VEHICLE_LATERAL_DYNAMICS_H
