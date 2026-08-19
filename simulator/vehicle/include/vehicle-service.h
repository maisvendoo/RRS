//------------------------------------------------------------------------------
//
//      Service system (снабжение локомотива: заправка топливом, маслом,
//      охлаждающей жидкостью, песком)
//      ТЗ "Система снабжения локомотива"
//
//      Заправка - физически протяжённый процесс: подключение рукава на
//      стоянке, порционная подача ресурса (литры по dt*rate), автоматическая
//      остановка при заполнении, обрыв при движении с подключённым
//      оборудованием. Ресурсы попадают в реальные системы ПЕ (дизель,
//      песочница) малыми порциями через их штатные API refuel/topUpCoolant/
//      refill - сигнатуры не меняются и остаются доступными сценариям.
//
//      Блокировка движения при подключении - isMovementBlocked(): потребитель
//      обрезает тягу (тот же паттерн, что у деповского питания).
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_SERVICE_H
#define     VEHICLE_SERVICE_H

#include    <QString>

#include    <cstddef>

class DieselEngineSystem;
class SandSystem;

//------------------------------------------------------------------------------
/// Система снабжения единицы ПС (заправочные колонки депо/ПТО)
//------------------------------------------------------------------------------
class ServiceSystem
{
public:

    /// Состояние процесса заправки (ТЗ, п.10)
    enum class State
    {
        Disconnected = 0,   ///< Рукава не подключены
        Connected = 1,      ///< Рукав подключён (стоянка!)
        Refueling = 2,      ///< Идёт подача ресурса
        Completed = 3,      ///< Заполнено, подача остановлена
        Aborted = 4         ///< Отмена: обрыв при движении / ошибка
    };

    /// Тип ресурса (ТЗ, п.1)
    enum class Resource
    {
        Fuel = 0,       ///< Дизельное топливо, л
        Oil = 1,        ///< Моторное масло, л
        Coolant = 2,    ///< Охлаждающая жидкость, л
        Sand = 3        ///< Песок, кг
    };

    ServiceSystem() = default;

    /// Загрузка секции [Service]
    void loadConfig(QString cfg_path);

    /// Подключить заправочный рукав ресурса (строка "Fuel"/"Oil"/
    /// "Coolant"/"Sand"). Разрешено только на стоянке (|v| < 0.3 м/с).
    /// После отмены (Aborted) подключение разрешено повторно
    bool connectService(const QString& resource);

    /// Отключить рукав / досрочно прекратить заправку
    void disconnectService();

    /// ПЕ находится в зоне заправочной колонки (задача модели маршрута
    /// по конфигу service.conf): подключение возможно только в зоне,
    /// выезд из зоны с рукавом - обрыв
    void setInZone(bool in_zone);

    /// Шаг: dt - время, velocity - скорость ПЕ; diesel и sand - реальные
    /// системы-приёмники ресурса (порционная подача через их API)
    void step(double dt, double velocity,
              DieselEngineSystem& diesel, SandSystem& sand);

    //--------- Состояние (для UI/сценариев) ---------

    State getState() const;
    QString getStateString() const;

    Resource getResource() const;
    QString getResourceString() const;

    /// Прогресс заправки 0..1 (доля от недостающего до полного)
    double getProgress() const;

    /// Движение заблокировано (рукав подключён / идёт подача)
    bool isMovementBlocked() const;

    /// Передано ресурса с начала текущей операции, л (песок - кг)
    double getDelivered() const;

    /// Система включена конфигом
    bool isEnabled() const;

    /// Последняя ошибка блокировки (подсказка игроку)
    QString getLastError() const;

    QString getDebugMsg() const;

private:

    /// Текущий уровень выбранного ресурса 0..1
    double currentLevel(DieselEngineSystem& diesel, SandSystem& sand) const;

    /// Ресурс доступен на этой ПЕ (топливо/масло/ОЖ - нужен дизель,
    /// песок - настроенная песочница)
    bool resourceAvailable(Resource resource,
                           DieselEngineSystem& diesel,
                           SandSystem& sand) const;

    /// Подать порцию ресурса (л или кг) в систему-приёмник
    void deliverPortion(double portion,
                        DieselEngineSystem& diesel,
                        SandSystem& sand) const;

    bool enabled = true;

    /// Скорости подачи: л/мин (песок - кг/мин)
    double fuel_rate = 200.0;
    double oil_rate = 20.0;
    double coolant_rate = 10.0;
    double sand_rate = 300.0;

    /// Порог скорости "движения" для блокировок, м/с
    double move_threshold = 0.3;

    State state = State::Disconnected;
    Resource resource = Resource::Fuel;

    /// Уровень ресурса на момент подключения (база прогресса)
    double initial_level = 0.0;

    /// Последний измеренный уровень ресурса 0..1
    double last_level = 0.0;

    /// Передано за текущую операцию, л (кг)
    double delivered = 0.0;

    /// Последняя известная скорость ПЕ (проверка стоянки при подключении)
    double last_velocity = 0.0;

    /// ПЕ в зоне заправочной колонки (координаты зон - модель маршрута)
    bool in_zone = false;

    QString last_error = "";
};

#endif // VEHICLE_SERVICE_H
