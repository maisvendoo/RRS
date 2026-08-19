//------------------------------------------------------------------------------
//
//      Pantograph system (токоприёмник)
//      ТЗ "Реалистичная контактная сеть + подстанции", п.14-21
//
//      Модель контакта полоза с проводом: прижимная сила (статическая +
//      аэродинамический подъём от скорости) против реакции провода;
//      на высокой скорости и при сильном ветре контакт может теряться -
//      дуга, искрение, перерывы питания. Износ полоза от силы и пробега.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_PANTOGRAPH_H
#define     VEHICLE_PANTOGRAPH_H

#include    <QString>

#include    <cstddef>

//------------------------------------------------------------------------------
/// Токоприёмник ПЕ
//------------------------------------------------------------------------------
class PantographSystem
{
public:

    PantographSystem() = default;

    /// Загрузка секции [Pantograph]
    void loadConfig(QString cfg_path);

    /// Поднять / опустить
    void setRaised(bool raised);
    bool isRaised() const;

    /// Шаг: скорость ПЕ, скорость ветра (м/с), напряжение на проводе (В)
    void step(double dt, double speed, double wind_speed, double wire_voltage);

    /// Контакт с проводом устойчив
    bool isContactOk() const;

    /// Прижимная сила, Н
    double getContactForce() const;

    /// Число дуг (потерь контакта) за последние ~1 с, 1/с
    double getArcRate() const;

    /// Износ угольной вставки 0..1
    double getWear() const;

    QString getDebugMsg() const;

private:

    bool raised = false;

    /// Статический прижим, Н
    double static_force = 70.0;

    /// Аэродинамический подъёмный коэффициент, Н/(м/с)^2
    double aero_coeff = 0.09;

    /// Реакция провода (жёсткость подвеса), Н/мм прижима сверх равновесного
    double wire_reaction = 25.0;

    /// Порог силы, ниже которого контакт неустойчив, Н
    double min_contact_force = 20.0;

    /// Скорость, с которой ветер сбивает полоз, м/с (боковой эффект)
    double wind_sensitivity = 0.02;

    /// Контакт
    bool contact_ok = false;
    double contact_force = 0.0;

    /// Дуги
    unsigned long arcs_total = 0;
    double arc_window = 0.0;    ///< с дуг за последнее окно
    double arc_timer = 0.0;
    double arc_rate = 0.0;

    double wear = 0.0;
};

#endif // VEHICLE_PANTOGRAPH_H
