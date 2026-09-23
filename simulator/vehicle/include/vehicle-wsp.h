//------------------------------------------------------------------------------
//
//      Wheel Slide Protection (противоюзная система, WSP/УВЗОС)
//      ТЗ "Сцепление колёс с рельсами", п.10-11
//
//      При юзе (блокировке колеса при торможении) сцепление падает и
//      тормозной путь растёт; система модулирует тормозной момент оси
//      возвратно-постоянной отсечкой: период ~0.3 с, внутри цикла фаза
//      сброса (момент оси = 0, колесо разгоняется до сцепления) и фаза
//      приложения. Скважность фазы сброса растёт со скоростью нарастания
//      юза: чем быстрее блокируется колесо, тем дольше держится сброс.
//      Отсечка НЕ мутирует Q_r: система выдаёт множитель 0..1 на ось,
//      применяемый при чтении тормозного момента в ОДУ (по образцу
//      brake_fade_eff). Гейт по скорости: ниже 5 км/ч не работает
//      (стояночную отсечку момента у неподвижного колеса не трогаем).
//
//      Конфиг ПЕ, секция [WSP]: Enabled (умолчание false),
//      SlipThreshold - порог скорости юза, м/с (умолчание 1.5),
//      Period - период модуляции, с (умолчание 0.3)
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_WSP_H
#define     VEHICLE_WSP_H

#include    <QString>

#include    <algorithm>
#include    <cmath>
#include    <cstddef>
#include    <vector>

//------------------------------------------------------------------------------
/// Противоюзная модуляция тормозного момента осей
//------------------------------------------------------------------------------
class WSPSystem
{
public:

    WSPSystem() = default;

    /// Загрузка секции [WSP] конфига ПЕ (реализация - vehicle.cpp)
    void loadConfig(QString cfg_path, std::size_t num_axis);

    /// Система включена конфигом
    bool isEnabled() const
    {
        return enabled;
    }

    /// Шаг системы.
    /// dt - шаг модели, с; velocity - скорость ПЕ, м/с;
    /// wheel_omega - угловые скорости осей, рад/с;
    /// wheel_radius - радиусы колёс, м (Q_r не нужен: юз виден по
    /// разности окружной скорости колеса и скорости ПЕ)
    void step(double dt,
              double velocity,
              const std::vector<double>& wheel_omega,
              const std::vector<double>& wheel_radius)
    {
        const std::size_t n = axle_phase.size();

        if (!enabled || n == 0 || dt <= 0.0)
            return;

        const double abs_v = std::abs(velocity);

        // Гейт скорости: ниже 5 км/ч не модулируем (стояночная
        // отсечка у неподвижного колеса - вне зоны действия WSP)
        if (abs_v <= speed_gate)
        {
            std::fill(axle_releasing.begin(), axle_releasing.end(), false);
            std::fill(axle_eff.begin(), axle_eff.end(), 1.0);

            for (double& slip : prev_slip)
                slip = 0.0;

            return;
        }

        for (std::size_t i = 0; i < n; ++i)
        {
            const double radius = (i < wheel_radius.size())
                    ? wheel_radius[i] : 0.475;
            const double omega = (i < wheel_omega.size())
                    ? wheel_omega[i] : 0.0;

            // Скорость юза: колесо тормозится медленнее катится,
            // окружная скорость ниже скорости ПЕ
            const double slip = abs_v - omega * radius;
            const double slip_rate = (slip - prev_slip[i]) / dt;

            prev_slip[i] = slip;

            if (!axle_releasing[i])
            {
                // Контроль юза: превышение порога скорости скольжения
                if (slip > slip_threshold)
                {
                    // Начало цикла отсечки. Скважность фазы сброса -
                    // от скорости нарастания юза: быстрое блокирование
                    // (лёд, грязь) требует длинного сброса
                    axle_releasing[i] = true;
                    axle_phase[i] = 0.0;
                    axle_duty_off[i] = std::min(0.35 + 0.08 * std::abs(slip_rate), 0.9);
                }
                else
                {
                    axle_eff[i] = 1.0;
                    continue;
                }
            }

            // Цикл модуляции: [0, duty*period) - сброс (eff = 0),
            // [duty*period, period) - приложение тормоза (eff = 1)
            axle_phase[i] += dt;

            const double off_time = axle_duty_off[i] * period;

            if (axle_phase[i] >= period)
            {
                // Цикл закончен: сцепление восстановлено (юз ниже
                // половины порога) - выходим из модуляции, иначе
                // начинаем новый цикл
                if (slip < 0.5 * slip_threshold)
                {
                    axle_releasing[i] = false;
                    axle_eff[i] = 1.0;
                }
                else
                {
                    axle_phase[i] = 0.0;
                }
            }
            else
            {
                axle_eff[i] = (axle_phase[i] < off_time) ? 0.0 : 1.0;
            }
        }
    }

    /// Множитель тормозного момента оси (0..1): 0 - фаза сброса WSP.
    /// Без системы/вне юза = 1.0
    double getAxleEfficiency(std::size_t axle) const
    {
        return (axle < axle_eff.size()) ? axle_eff[axle] : 1.0;
    }

    /// Ось в цикле модуляции (диагностика, звук WSP)
    bool isAxleCycling(std::size_t axle) const
    {
        return (axle < axle_releasing.size()) ? axle_releasing[axle] : false;
    }

private:

    bool enabled = false;

    /// Порог скорости юза (окружная ниже поступательной), м/с
    double slip_threshold = 1.5;

    /// Период модуляции тормозного момента, с
    double period = 0.3;

    /// Гейт скорости: ниже 5 км/ч WSP не работает, м/с
    double speed_gate = 5.0 / 3.6;

    std::vector<double> axle_phase;     ///< Фаза цикла оси, с
    std::vector<bool> axle_releasing;   ///< Ось в цикле отсечки
    std::vector<double> axle_duty_off;  ///< Скважность фазы сброса 0..1
    std::vector<double> prev_slip;      ///< Скорость юза прошлого шага
    std::vector<double> axle_eff;       ///< Множитель момента 0..1
};

#endif // VEHICLE_WSP_H
