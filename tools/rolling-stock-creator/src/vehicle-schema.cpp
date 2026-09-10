#include "vehicle-schema.h"

#include <QCoreApplication>

//------------------------------------------------------------------------------
//
//  Схема секций конфигурации подвижного состава.
//  Значения min/max и дефолты соответствуют диапазонам, которые
//  использует симулятор при чтении этих секций (simulator/vehicle).
//
//------------------------------------------------------------------------------

namespace
{

FieldSpec real(const QString& key, double min_value, double max_value,
               const QString& default_value, const QString& description,
               bool required = false)
{
    return FieldSpec{key, FieldType::Double, min_value, max_value,
                     default_value, description, required};
}

FieldSpec integer(const QString& key, int min_value, int max_value,
                  const QString& default_value, const QString& description,
                  bool required = false)
{
    return FieldSpec{key, FieldType::Int, static_cast<double>(min_value),
                     static_cast<double>(max_value), default_value,
                     description, required};
}

FieldSpec text(const QString& key, const QString& default_value,
               const QString& description, bool required = false)
{
    return FieldSpec{key, FieldType::String, 0.0, 0.0, default_value,
                     description, required};
}

FieldSpec boolean(const QString& key, bool default_value,
                  const QString& description)
{
    return FieldSpec{key, FieldType::Bool, 0.0, 0.0,
                     default_value ? "true" : "false", description, false};
}

} // namespace

const std::vector<SectionSpec>& vehicleSchema()
{
    static const std::vector<SectionSpec> schema = {
        {
            "Vehicle", false,
            {
                real ("EmptyMass",       0.0,   10000000.0, "40000",   QCoreApplication::translate("Schema", "Масса тары, кг"), true),
                real ("PayloadMass",     0.0,   10000000.0, "60000",   QCoreApplication::translate("Schema", "Масса груза, кг"), true),
                real ("Length",          1.0,   100.0,      "15.0",    QCoreApplication::translate("Schema", "Длина по осям автосцепок, м"), true),
                integer("NumAxis",       2,     16,         "4",       QCoreApplication::translate("Schema", "Число осей"), true),
                real ("WheelDiameter",   0.3,   2.5,        "0.95",    QCoreApplication::translate("Schema", "Диаметр колеса, м"), true),
                real ("MaxSpeed",        1.0,   600.0,      "100.0",   QCoreApplication::translate("Schema", "Конструкционная (поездная) скорость, км/ч")),
                real ("WheelInertia",    0.0,   10000.0,    "100.0",   QCoreApplication::translate("Schema", "Момент инерции колеса, кг*м^2")),
                text ("MainResist",      "default", QCoreApplication::translate("Schema", "Конфиг основного сопротивления движению")),
                text ("WheelRailFriction", "default", QCoreApplication::translate("Schema", "Конфиг сцепления колеса с рельсом")),
                real ("ReductorCoeff",   0.1,   100.0,     "1.0",     QCoreApplication::translate("Schema", "Передаточное число тягового редуктора")),
                text ("SoundDir",        "",    QCoreApplication::translate("Schema", "Каталог звуков ПЕ")),
                text ("CouplingModule",  "sa3", QCoreApplication::translate("Schema", "Модуль сцепного устройства")),
                text ("CouplingConfig",  "sa3", QCoreApplication::translate("Schema", "Конфиг сцепного устройства")),
                text ("AirDistModule",   "vr483", QCoreApplication::translate("Schema", "Модуль воздухораспределителя")),
                text ("AirDistConfig",   "vr483", QCoreApplication::translate("Schema", "Конфиг воздухораспределителя")),
                text ("BrakeMechConfig", "",    QCoreApplication::translate("Schema", "Конфиг тормозной механики")),
            }
        },
        {
            "Suspension", false,
            {
                boolean("Enabled", true, QCoreApplication::translate("Schema", "Включить модель вертикальной динамики")),
                real ("PrimaryStiffness",   1.0e4, 1.0e8, "1200000.0", QCoreApplication::translate("Schema", "Жёсткость первой ступени, Н/м")),
                real ("PrimaryDamping",     0.0,   1.0e6, "25000.0",   QCoreApplication::translate("Schema", "Демпфирование первой ступени, Н*с/м")),
                real ("PrimaryStroke",      0.005, 0.5,   "0.06",      QCoreApplication::translate("Schema", "Ход первой ступени, м")),
                real ("SecondaryStiffness", 1.0e4, 1.0e8, "450000.0",  QCoreApplication::translate("Schema", "Жёсткость второй ступени, Н/м")),
                real ("SecondaryDamping",   0.0,   1.0e6, "18000.0",   QCoreApplication::translate("Schema", "Демпфирование второй ступени, Н*с/м")),
                real ("SecondaryStroke",    0.005, 0.5,   "0.09",      QCoreApplication::translate("Schema", "Ход второй ступени, м")),
                real ("WheelsetMass",       100.0, 20000.0, "1400.0",  QCoreApplication::translate("Schema", "Масса колёсной пары, кг")),
                real ("BogieMass",          100.0, 20000.0, "2200.0",  QCoreApplication::translate("Schema", "Масса тележки, кг")),
                real ("ContactStiffness",   1.0e6, 1.0e10, "1.0e9",    QCoreApplication::translate("Schema", "Жёсткость контакта колесо-рельс, Н/м")),
                real ("ContactDamping",     0.0,   1.0e7,  "2.0e5",    QCoreApplication::translate("Schema", "Демпфирование контакта, Н*с/м")),
                real ("BogieOffset",        0.0,   15.0,   "0.0",      QCoreApplication::translate("Schema", "Смещение тележек от центра, м")),
                real ("WheelsetSpacing",    1.0,   5.0,    "1.85",     QCoreApplication::translate("Schema", "База тележки (расстояние между осями), м")),
                real ("SecondaryHalfSpan",  0.5,   10.0,   "1.2",      QCoreApplication::translate("Schema", "Полубаза второй ступени, м")),
                real ("PitchRadius",        0.1,   10.0,   "0.29",     QCoreApplication::translate("Schema", "Радиус инерции подрессоренных масс (галопирование), м")),
                real ("RollRadius",         0.1,   10.0,   "1.15",     QCoreApplication::translate("Schema", "Радиус инерции (боковая качка), м")),
                real ("Substep",            0.0002, 0.005, "0.001",    QCoreApplication::translate("Schema", "Шаг интегрирования подвески, с")),
                integer("NumBogies",        1,     8,      "2",        QCoreApplication::translate("Schema", "Число тележек")),
            }
        },
        {
            "LateralDynamics", false,
            {
                boolean("Enabled", true, QCoreApplication::translate("Schema", "Включить модель боковой динамики")),
                real ("Conicity",               0.0,  1.0,    "0.1",     QCoreApplication::translate("Schema", "Конусность поверхности катания")),
                real ("FlangeClearance",        0.0,  0.1,    "0.023",   QCoreApplication::translate("Schema", "Зазор между гребнем и рельсом, м")),
                real ("FlangeStiffness",        1.0e4, 1.0e8, "5.0e7",   QCoreApplication::translate("Schema", "Жёсткость контакта гребня, Н/м")),
                real ("FlangeDamping",          0.0,  1.0e6,  "2.0e5",   QCoreApplication::translate("Schema", "Демпфирование контакта гребня, Н*с/м")),
                real ("FlangeAngle",            45.0, 90.0,   "68.0",    QCoreApplication::translate("Schema", "Угол наклона гребня, град")),
                real ("CreepLongitudinal",      0.0,  1.0,    "0.1",     QCoreApplication::translate("Schema", "Коэффициент продольного крипа")),
                real ("CreepLateral",           0.0,  1.0,    "0.1",     QCoreApplication::translate("Schema", "Коэффициент поперечного крипа")),
                real ("PrimaryLateralStiffness", 1.0e4, 1.0e8, "3.0e7",  QCoreApplication::translate("Schema", "Боковая жёсткость первой ступени, Н/м")),
                real ("PrimaryLateralDamping",  0.0,  1.0e6,  "3.0e5",   QCoreApplication::translate("Schema", "Боковое демпфирование первой ступени, Н*с/м")),
                real ("PrimaryYawStiffness",    1.0e4, 1.0e8, "1.0e7",   QCoreApplication::translate("Schema", "Жёсткость виляния первой ступени, Н*м/рад")),
                real ("PrimaryYawFreePlay",     0.0,  0.05,   "0.002",   QCoreApplication::translate("Schema", "Свободный ход в уровне первой ступени, м")),
                real ("BogieYawStiffness",      0.0,  1.0e8,  "5.0e6",   QCoreApplication::translate("Schema", "Жёсткость виляния тележки, Н*м/рад")),
                real ("BogieYawFreePlay",       0.0,  0.05,   "0.0",     QCoreApplication::translate("Schema", "Свободный ход виляния тележки, рад")),
                real ("SecondaryLateralStiffness", 1.0e4, 1.0e8, "4.0e6", QCoreApplication::translate("Schema", "Боковая жёсткость второй ступени, Н/м")),
                real ("SecondaryLateralDamping",  0.0, 1.0e6,  "6.0e5",  QCoreApplication::translate("Schema", "Боковое демпфирование второй ступени, Н*с/м")),
                real ("SideBearerClearance",    0.0,  0.1,    "0.02",    QCoreApplication::translate("Schema", "Зазор скользуна, м")),
                real ("SideBearerStiffness",    1.0e4, 1.0e8,  "1.0e8",  QCoreApplication::translate("Schema", "Жёсткость скользуна, Н/м")),
                real ("WheelsetMass",           100.0, 20000.0, "1400.0", QCoreApplication::translate("Schema", "Масса колёсной пары, кг")),
                real ("BogieMass",              100.0, 20000.0, "2200.0", QCoreApplication::translate("Schema", "Масса тележки, кг")),
                real ("WheelsetYawInertia",     100.0, 1.0e6,  "6000.0", QCoreApplication::translate("Schema", "Момент инерции КП (виляние), кг*м^2")),
                real ("BogieYawInertia",        100.0, 1.0e6,  "8000.0", QCoreApplication::translate("Schema", "Момент инерции тележки (виляние), кг*м^2")),
                real ("BodyYawInertia",         1000.0, 1.0e8, "1.0e6",   QCoreApplication::translate("Schema", "Момент инерции кузова (виляние), кг*м^2")),
                real ("BogieOffset",            0.0,  15.0,   "0.0",     QCoreApplication::translate("Schema", "Смещение тележек от центра, м")),
                real ("WheelsetSpacing",        1.0,  5.0,    "1.85",    QCoreApplication::translate("Schema", "База тележки, м")),
                real ("Substep",                0.0002, 0.005, "0.001",  QCoreApplication::translate("Schema", "Шаг интегрирования, с")),
                integer("NumBogies",            1,    8,      "2",       QCoreApplication::translate("Schema", "Число тележек")),
            }
        },
        {
            "Derailment", false,
            {
                boolean("Enabled", true, QCoreApplication::translate("Schema", "Включить модель схода с рельсов")),
                real ("YQLimit",              0.1,  10.0,  "0.8",    QCoreApplication::translate("Schema", "Критерий Надал Y/Q")),
                real ("WheelUnloadLimit",     0.1,  1.0,   "0.6",    QCoreApplication::translate("Schema", "Предел разгрузки колеса (доля от статической нагрузки)")),
                real ("FlangeClimbTime",      0.01, 10.0,  "0.05",   QCoreApplication::translate("Schema", "Время подъёма на гребень до схода, с")),
                real ("DerailDisplacement",   0.05, 5.0,   "0.5",    QCoreApplication::translate("Schema", "Боковое смещение, после которого ПЕ считается сошедшей, м")),
                real ("DerailedResistance",   0.0,  1000.0, "100.0", QCoreApplication::translate("Schema", "Удельное сопротивление после схода, Н/кН")),
                real ("LateralImpactThreshold", 1.0, 1.0e7, "5.0e5", QCoreApplication::translate("Schema", "Порог бокового удара, Н")),
                real ("MassCenterHeight",     0.1,  5.0,   "1.8",    QCoreApplication::translate("Schema", "Высота центра масс, м")),
                real ("RolloverArm",          0.1,  5.0,   "1.0",    QCoreApplication::translate("Schema", "Плечо опрокидывания, м")),
                real ("ResistanceSleepers",   0.0,  1.0e6, "5.0e4",  QCoreApplication::translate("Schema", "Сопротивление шпал, Н")),
                real ("ResistanceBallast",    0.0,  1.0e6, "1.5e5",  QCoreApplication::translate("Schema", "Сопротивление балласта, Н")),
                real ("ResistanceGround",     0.0,  1.0e6, "3.0e5",  QCoreApplication::translate("Schema", "Сопротивление грунта, Н")),
                real ("SleeperSpacing",       0.2,  2.0,   "0.54",   QCoreApplication::translate("Schema", "Шаг шпал, м")),
            }
        },
        {
            "DamageSystem", false,
            {
                real ("BodyBreakEnergy",    1.0e3, 1.0e8, "5.0e5", QCoreApplication::translate("Schema", "Энергия разрушения кузова, Дж")),
                real ("BogieBreakEnergy",   1.0e3, 1.0e8, "8.0e5", QCoreApplication::translate("Schema", "Энергия разрушения тележки, Дж")),
                real ("CouplerBreakEnergy", 1.0e3, 1.0e8, "4.0e5", QCoreApplication::translate("Schema", "Энергия разрушения сцепки, Дж")),
                real ("BrakeBreakEnergy",   1.0e3, 1.0e8, "3.0e5", QCoreApplication::translate("Schema", "Энергия разрушения тормозного оборудования, Дж")),
                real ("TankBreakEnergy",    1.0e3, 1.0e8, "6.0e5", QCoreApplication::translate("Schema", "Энергия разрушения топливного бака, Дж")),
                real ("ImpactThreshold",    1.0,   1.0e8, "1.0e4", QCoreApplication::translate("Schema", "Порог энергии удара, ниже которого повреждений нет, Дж")),
                real ("DerailWearRate",     0.0,   10.0,  "1.0",   QCoreApplication::translate("Schema", "Скорость износа при движении после схода")),
            }
        },
        {
            // Секция читается VehicleCollision::loadConfig
            // (simulator/vehicle/src/vehicle-collision.cpp)
            "Collision", false,
            {
                boolean("Enabled",           true, QCoreApplication::translate("Schema", "Коллайдеры включены")),
                real ("BodyHalfLength",      0.1, 50.0, "6.96",  QCoreApplication::translate("Schema", "Полудлина кузова, м")),
                real ("BodyHalfWidth",       0.1, 5.0,  "1.55",  QCoreApplication::translate("Schema", "Полуширина кузова, м")),
                real ("BodyHalfHeight",      0.1, 5.0,  "1.6",   QCoreApplication::translate("Schema", "Полувысота кузова, м")),
                real ("BodyOffsetZ",         0.0, 6.0,  "2.0",   QCoreApplication::translate("Schema", "Высота центра кузова над УГР, м")),
                integer("NumBogies",         0, 8,      "2",     QCoreApplication::translate("Schema", "Число тележек с коллайдерами")),
                real ("BogieHalfLength",     0.1, 10.0, "1.8",   QCoreApplication::translate("Schema", "Полудлина тележки, м")),
                real ("BogieHalfWidth",      0.1, 3.0,  "1.15",  QCoreApplication::translate("Schema", "Полуширина тележки, м")),
                real ("BogieHalfHeight",     0.05, 2.0, "0.5",   QCoreApplication::translate("Schema", "Полувысота тележки, м")),
                real ("BogieOffset",         0.0, 15.0, "4.86",  QCoreApplication::translate("Schema", "От центра ПЕ до крайней тележки, м")),
                real ("BogieOffsetZ",        0.0, 3.0,  "0.85",  QCoreApplication::translate("Schema", "Высота центра тележки над УГР, м")),
                real ("WheelsetSpacing",     0.0, 5.0,  "2.4",   QCoreApplication::translate("Schema", "Межосевое расстояние в тележке, м")),
                real ("WheelsetHalfWidth",   0.05, 2.0, "1.0",   QCoreApplication::translate("Schema", "Полуширина колёсной пары, м")),
                real ("WheelRadius",         0.1, 1.5,  "0.475", QCoreApplication::translate("Schema", "Радиус колеса, м")),
            }
        },
        {
            "MassCenter", false,
            {
                real ("Height",       0.0, 5.0,   "1.8", QCoreApplication::translate("Schema", "Высота центра масс, м")),
                real ("Longitudinal", -15.0, 15.0, "0.0", QCoreApplication::translate("Schema", "Продольное смещение центра масс, м")),
                real ("Lateral",      -2.0, 2.0,   "0.0", QCoreApplication::translate("Schema", "Поперечное смещение центра масс, м")),
            }
        },
        {
            "Sand", false,
            {
                boolean("Enabled", false, QCoreApplication::translate("Schema", "Наличие песочной системы")),
                boolean("AutoMode", false, QCoreApplication::translate("Schema", "Автоматическая подача песка при боксовании")),
                real ("Capacity",        1.0,  10000.0, "1000.0", QCoreApplication::translate("Schema", "Ёмкость бункера песка, кг")),
                real ("Amount",          0.0,  10000.0, "500.0",  QCoreApplication::translate("Schema", "Текущее количество песка, кг")),
                real ("Moisture",        0.0,  1.0,     "0.1",    QCoreApplication::translate("Schema", "Влажность песка (0-1)")),
                real ("BaseRate",        0.01, 50.0,    "2.0",    QCoreApplication::translate("Schema", "Базовый расход песка, кг/с")),
                real ("WetFlowLimit",    0.0,  50.0,    "0.5",    QCoreApplication::translate("Schema", "Предел подачи мокрого песка, кг/с")),
                real ("SlipThreshold",   0.0,  1.0,     "0.05",   QCoreApplication::translate("Schema", "Порог проскальзывания для автопеска")),
                real ("ActivationDelay", 0.0,  30.0,    "1.0",    QCoreApplication::translate("Schema", "Задержка включения подачи, с")),
                real ("ReleaseDelay",    0.0,  30.0,    "2.0",    QCoreApplication::translate("Schema", "Задержка выключения подачи, с")),
                real ("SandBoost",       0.0,  1.0,     "0.15",   QCoreApplication::translate("Schema", "Прирост сцепления от песка")),
            }
        },
        {
            "Diesel", false,
            {
                real ("NominalPower",  10.0,   10000.0, "1500.0", QCoreApplication::translate("Schema", "Номинальная мощность, кВт")),
                real ("IdleRPM",       100.0,  1500.0,  "400.0",  QCoreApplication::translate("Schema", "Обороты холостого хода, об/мин")),
                real ("MaxRPM",        300.0,  3000.0,  "1100.0", QCoreApplication::translate("Schema", "Максимальные обороты, об/мин")),
                real ("FuelCapacity",  10.0,   50000.0, "5000.0", QCoreApplication::translate("Schema", "Ёмкость топливного бака, л")),
                real ("Fuel",          0.0,    50000.0, "3000.0", QCoreApplication::translate("Schema", "Текущий запас топлива, л")),
                real ("OilCapacity",   5.0,    5000.0,  "300.0",  QCoreApplication::translate("Schema", "Ёмкость масляной системы, л")),
                real ("Oil",           0.0,    5000.0,  "250.0",  QCoreApplication::translate("Schema", "Текущий запас масла, л")),
                real ("TempWarm",      20.0,   100.0,   "60.0",   QCoreApplication::translate("Schema", "Температура прогретого дизеля, град C")),
                real ("TempHigh",      40.0,   120.0,   "85.0",   QCoreApplication::translate("Schema", "Повышенная температура, град C")),
                real ("TempOverheat",  50.0,   150.0,   "95.0",   QCoreApplication::translate("Schema", "Температура перегрева, град C")),
                real ("TempCritical",  60.0,   200.0,   "105.0",  QCoreApplication::translate("Schema", "Критическая температура, град C")),
                real ("IdleFuelRate",  0.1,    100.0,   "5.0",    QCoreApplication::translate("Schema", "Расход топлива на холостом ходу, кг/ч")),
                real ("SFC0",          0.01,   1.0,     "0.21",   QCoreApplication::translate("Schema", "Удельный расход при 0% мощности, кг/(кВт*ч)")),
                real ("SFC25",         0.01,   1.0,     "0.20",   QCoreApplication::translate("Schema", "Удельный расход при 25% мощности, кг/(кВт*ч)")),
                real ("SFC50",         0.01,   1.0,     "0.198",  QCoreApplication::translate("Schema", "Удельный расход при 50% мощности, кг/(кВт*ч)")),
                real ("SFC75",         0.01,   1.0,     "0.203",  QCoreApplication::translate("Schema", "Удельный расход при 75% мощности, кг/(кВт*ч)")),
                real ("SFC100",        0.01,   1.0,     "0.215",  QCoreApplication::translate("Schema", "Удельный расход при 100% мощности, кг/(кВт*ч)")),
            }
        },
        {
            "Energy", false,
            {
                text ("DriveType",         "electric", QCoreApplication::translate("Schema", "Тип привода: electric / diesel / hybrid")),
                real ("TractionEfficiency", 0.1, 1.0,    "0.85", QCoreApplication::translate("Schema", "КПД тягового режима")),
                real ("RegenEfficiency",    0.0, 1.0,    "0.75", QCoreApplication::translate("Schema", "КПД рекуперации")),
                real ("AuxPower",           0.0, 1000.0, "50.0", QCoreApplication::translate("Schema", "Мощность вспомогательных цепей, кВт")),
                real ("NominalVoltage",     500.0, 50000.0, "3000.0", QCoreApplication::translate("Schema", "Номинальное напряжение, В")),
                real ("CurrentLimit",       10.0, 5000.0,  "1000.0", QCoreApplication::translate("Schema", "Предел тока, А")),
                real ("MaxRegenPower",      0.0, 10000.0, "3000.0", QCoreApplication::translate("Schema", "Предел мощности рекуперации, кВт")),
                real ("MinRegenSpeed",      0.0, 100.0,   "5.0",  QCoreApplication::translate("Schema", "Минимальная скорость рекуперации, км/ч")),
                real ("ContinuousPower",    0.0, 20000.0, "4000.0", QCoreApplication::translate("Schema", "Длительная мощность, кВт")),
            }
        },
        {
            "CargoWagon", false,
            {
                real ("MaxLoad",        0.0, 1000000.0, "60000.0", QCoreApplication::translate("Schema", "Грузоподъёмность, кг")),
                real ("CurrentLoad",    0.0, 1000000.0, "0.0",     QCoreApplication::translate("Schema", "Текущая загрузка, кг")),
                text ("Type",           "coal", QCoreApplication::translate("Schema", "Тип груза")),
                real ("COMLongitudinal", -15.0, 15.0,   "0.0",     QCoreApplication::translate("Schema", "Смещение центра масс груза по длине, м")),
                real ("COMLateral",      -2.0,  2.0,    "0.0",     QCoreApplication::translate("Schema", "Смещение центра масс груза по ширине, м")),
                text ("AllowedCargo",   "", QCoreApplication::translate("Schema", "Список разрешённых грузов через запятую")),
            }
        },
        {
            "PassengerCar", false,
            {
                integer("Capacity",         1, 500,  "56",   QCoreApplication::translate("Schema", "Вместимость, мест")),
                integer("CurrentPassengers", 0, 500,  "0",    QCoreApplication::translate("Schema", "Текущее число пассажиров")),
                integer("Doors",             1, 10,   "2",    QCoreApplication::translate("Schema", "Число дверей")),
                real ("DoorWidth",           0.5, 2.0, "1.0", QCoreApplication::translate("Schema", "Ширина двери, м")),
                real ("PassRate",            0.1, 5.0, "1.0", QCoreApplication::translate("Schema", "Скорость посадки, чел/с")),
            }
        },
        {
            "CabElement", true,
            {
                integer("ControlID", 1, 10000, "1",    QCoreApplication::translate("Schema", "Идентификатор органа управления")),
                text ("Name",        "", QCoreApplication::translate("Schema", "Название органа управления")),
                text ("Hint",        "", QCoreApplication::translate("Schema", "Подсказка")),
                boolean("IsToggle",  false, QCoreApplication::translate("Schema", "Переключатель (иначе — кнопка)")),
            }
        },
        {
            "SME", false,
            {
                integer("GroupId", 0, 100, "0", QCoreApplication::translate("Schema", "Номер группы системы многих единиц")),
                boolean("IsLead",  false, QCoreApplication::translate("Schema", "Ведущая единица в группе СМЕ")),
            }
        }
    };

    return schema;
}

const SectionSpec* findSectionSpec(const QString& name)
{
    for (const SectionSpec& section : vehicleSchema())
    {
        if (section.name == name)
        {
            return &section;
        }
    }

    return nullptr;
}
