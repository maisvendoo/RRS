//------------------------------------------------------------------------------
//
//      Vehicle vertical dynamics (multibody: wheelset - bogie - body)
//      ТЗ "Неровности пути": рельсовый профиль -> колёсная пара ->
//      первая ступень подвешивания -> тележка -> вторая ступень -> кузов
//
//      Модель в отклонениях от статического равновесия: гравитация и
//      статические выборки подвески взаимно сокращаются, входом являются
//      только неровности пути (z_rail != 0). Подвеска и контакт -
//      односторонние: потеря контакта колеса с рельсом детектируется.
//
//      Системы координат:
//      - x - вперёд по ПЕ (перед ПЕ - положительное направление);
//      - z - вверх;
//      - крен theta: положительный - левый борт поднимается;
//      - тангаж pitch: положительный - передняя часть поднимается.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_DYNAMICS_H
#define     VEHICLE_DYNAMICS_H

#include    <QString>

#include    <cstddef>
#include    <cstdint>
#include    <deque>
#include    <functional>
#include    <vector>

#ifndef VEHICLE_EXPORT
    #if defined(VEHICLE_LIB)
        #define VEHICLE_EXPORT Q_DECL_EXPORT
    #else
        #define VEHICLE_EXPORT Q_DECL_IMPORT
    #endif
#endif

//------------------------------------------------------------------------------
/// Вертикальная динамика единицы подвижного состава
//------------------------------------------------------------------------------
class VEHICLE_EXPORT VehicleVerticalDynamics
{
public:

    /// Источник высоты рельса: (абсолютная координата пути, сторона рельса)
    /// -> вертикальное смещение, м. Сторона: 0 - левая, 1 - правая
    using RailHeightFn = std::function<double(double, int)>;

    VehicleVerticalDynamics() = default;

    /// Загрузка параметров из секции [Suspension] конфига ПЕ.
    /// Умолчания геометрии выводятся из длины и числа осей
    void loadConfig(QString cfg_path,
                    double full_mass,
                    std::size_t num_axis,
                    double length);

    /// Вертикальная динамика включена конфигом
    bool isEnabled() const;

    /// Источник неровностей привязан (контроллер ПЕ на топологии)
    bool isReady() const;

    /// Привязка источника высоты рельса (вызывается один раз при расстановке)
    void setRailHeightSource(RailHeightFn fn);

    /// Статическое распределение нагрузки по осям (множители, среднее 1.0,
    /// от продольного смещения центра масс). Пустой вектор - равномерно
    void setStaticAxleLoads(const std::vector<double>& axle_shares);

    /// Вертикальный импульс на колёсную пару (ползун, удар о шпалу):
    /// добавляет скорость падения колесу, далее контакт Герца отбивает
    /// (ТЗ "Ползун", п.7: колесо -> букса -> тележка -> подвеска -> кузов)
    void applyFlatImpact(std::size_t axle, double drop_speed);

    /// Шаг вертикальной динамики.
    /// dt - шаг интегрирования модели, с;
    /// velocity - скорость ПЕ по координате пути, м/с;
    /// dir - ориентация ПЕ относительно поезда (+1/-1);
    /// train_coord - координата центра ПЕ (система координат поезда);
    /// full_mass - полная масса ПЕ с грузом, кг;
    /// cant_mm - возвышение наружного рельса, мм (Б16: разность высот
    /// рельсов задаёт статический крен колёсной пары/тележки через
    /// смещение сторон в контакте колёс; "+" - левый рельс выше);
    /// longitudinal_accel - продольное ускорение ПЕ, м/с^2 (тангаж
    /// кузова: клюёт носом при торможении, M = m*a*h_cm)
    void step(double dt, double velocity, std::int8_t dir,
              double train_coord, double full_mass,
              double cant_mm = 0.0,
              double longitudinal_accel = 0.0);

    //--------- Датчики для других систем движка ---------

    /// Вертикальное ускорение кузова (фильтрованное), м/с^2
    double getBodyAcceleration() const;

    /// Рывок (производная ускорения) кузова, м/с^3
    double getBodyJerk() const;

    /// Вертикальное перемещение кузова от статического равновесия, м
    double getBodyVerticalPosition() const;

    /// Тангаж кузова, рад
    double getBodyPitch() const;

    /// Крен кузова, рад
    double getBodyRoll() const;

    /// Вертикальное ускорение тележки, м/с^2
    double getBogieAcceleration(std::size_t bogie) const;

    /// Доля статической нагрузки на ось (1.0 - статика, 0 - контакта нет)
    double getAxleLoadFactor(std::size_t axle) const;

    /// Потеря контакта колёсной пары с рельсами
    bool isAxleUnloaded(std::size_t axle) const;

    //--------- Оценка качества езды (ТЗ, п.20-21) ---------

    /// Среднеквадратичное ускорение кузова за окно 5 с, м/с^2
    double getRMSAcceleration() const;

    /// Максимум ускорения с момента сброса, м/с^2
    double getMaxAcceleration() const;

    /// Максимум рывка с момента сброса, м/с^3
    double getMaxJerk() const;

    /// Число сильных ударов (ускорение выше 3 м/с^2) с момента сброса
    int getShockCount() const;

    /// Суммарное число зарегистрированных ударов осей о стыки/дефекты
    /// пути (счётчик монотонный; мост физика -> аудио: дельта между
    /// опросами = новые удары, событие JointImpact)
    unsigned long getJointImpactCount() const;

    /// Сила последнего удара оси о стык, 0..1 (нормированный заброс
    /// ускорения колёсной пары над порогом)
    double getLastJointImpactIntensity() const;

    /// Индекс плавности хода (приближение индекса Шперлинга без
    /// частотного взвешивания): Wz = 0.896 * a_rms^0.798
    double getRideIndex() const;

    /// Сброс метрик качества езды
    void resetComfortMetrics();

    /// Строка отладки: перемещения, ускорения, рывок, нагрузки осей
    QString getDebugMsg() const;

private:

    /// Параметры одной ступени подвешивания
    struct SuspensionStage
    {
        double stiffness = 0.0;     ///< Суммарная жёсткость ступени, Н/м
        double damping = 0.0;       ///< Суммарное демпфирование, Н*с/м
        double stroke = 0.06;       ///< Ход до упора, м
        double stop_stiffness = 0.0;///< Жёсткость упора, Н/м
    };

    /// Состояния степеней свободы (перемещение + скорость)
    struct Dof
    {
        double pos = 0.0;
        double vel = 0.0;
        double acc = 0.0;
    };

    void integrateSubstep(double h,
                          double velocity,
                          std::int8_t dir,
                          double train_coord,
                          double cant_mm,
                          double longitudinal_accel);

    /// Сила упоров ступени: за пределами хода включается жёсткий упор
    static double bumpStop(double deflection, const SuspensionStage& stage);

    bool enabled = true;
    bool ready = false;

    RailHeightFn rail_height;

    // Геометрия
    std::size_t num_axis = 4;
    std::size_t num_bogies = 2;
    double length = 14.0;
    std::vector<double> axle_offset;    ///< Смещение оси от центра ПЕ, м
    std::vector<std::size_t> axle_bogie;///< Индекс тележки оси
    std::vector<double> bogie_offset;   ///< Смещение тележки от центра, м

    // Массы, кг
    double full_mass = 25000.0;
    double wheelset_mass = 1400.0;      ///< Неподрессоренная масса на ось
    double bogie_mass = 2200.0;         ///< Рама тележки
    double body_mass = 18000.0;         ///< Подрессоренная часть (пересчёт каждый шаг)

    // Подвеска
    SuspensionStage primary;            ///< на ось (суммарно на две стороны)
    SuspensionStage secondary;          ///< на тележку
    double secondary_half_span = 0.95;  ///< Полубаза пружин II ступени, м
    double primary_half_span = 0.76;    ///< Полубаза пружин I ступени, м
    double contact_half_span = 0.76;    ///< Полубаза колёс (полуколея), м

    // Контакт колеса с рельсом (Герц, линейная аппроксимация)
    double contact_stiffness = 1.0e9;   ///< На колёсную пару, Н/м
    double contact_damping = 2.0e5;     ///< На колёсную пару, Н*с/м

    // Инерция кузова
    double pitch_radius = 4.0;          ///< Радиус инерции тангажа, м
    double roll_radius = 1.15;          ///< Радиус инерции крена, м

    /// Плечо инерционного тангажа, м: высота ЦМ кузова над центром
    /// букс (продольное ускорение a создаёт момент M = m*a*h_cm,
    /// кузов клюёт носом при торможении, ProdVertKoleb п.4)
    double pitch_torque_arm = 1.6;

    // Подшаг интегрирования, с (жёсткий контакт Герца требует малого шага)
    double substep = 0.001;

    // Состояния
    std::vector<Dof> wheelset_z;        ///< Подпрыгивание колёсных пар
    std::vector<Dof> wheelset_roll;     ///< Виляние крена колёсных пар
    std::vector<Dof> bogie_z;
    std::vector<Dof> bogie_roll;
    Dof body_z;
    Dof body_pitch;
    Dof body_roll;

    /// Рабочие буферы подшага (без аллокаций в цикле интегрирования)
    std::vector<double> rail_z;             ///< Высота рельса: ось x сторона
    std::vector<double> rail_rate;          ///< Скорость рельса: ось x сторона
    std::vector<double> contact_force;      ///< Сила контакта: ось x сторона
    std::vector<double> primary_on_axle;
    std::vector<double> primary_on_bogie;
    std::vector<double> primary_torque_on_axle;
    std::vector<double> primary_torque_on_bogie;
    std::vector<double> secondary_on_bogie;
    std::vector<double> secondary_torque_on_bogie;

    // Датчики
    std::vector<double> axle_load_factor;
    std::vector<double> prev_rail_z;    ///< Предыдущая высота рельса: ось x сторона

    /// Статическая нагрузка на ось (доля от полной), из центра масс
    std::vector<double> static_axle_share;
    double body_accel_filtered = 0.0;
    double body_jerk = 0.0;
    double prev_body_accel = 0.0;

    // Метрики качества езды
    double accel_sq_sum = 0.0;
    std::deque<double> accel_window;    ///< Окно ускорений ~5 с
    double max_accel = 0.0;
    double max_jerk = 0.0;
    int shock_count = 0;
    double shock_refractory = 0.0;      ///< Время до следующего возможного удара, с
    double metrics_sample_timer = 0.0;

    // Детектор ударов осей о стыки (звук JointImpact, ТЗ "43-47", п.6-8)
    double joint_impact_threshold = 25.0;   ///< Порог ускорения колёсной пары, м/с^2
    double joint_impact_refractory = 0.15;  ///< Рефрактерность детектора, с
    std::vector<unsigned long> axle_joint_count; ///< Счётчик ударов по осям
    std::vector<double> axle_joint_refract;      ///< Таймер рефрактерности, с
    double last_joint_intensity = 0.0;          ///< Сила последнего удара 0..1
};

#endif // VEHICLE_DYNAMICS_H
