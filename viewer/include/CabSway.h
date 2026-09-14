#ifndef CAB_SWAY_H
#define CAB_SWAY_H

#include <vsg/maths/vec3.h>

//------------------------------------------------------------------------------
//
//      Раскачка кабины в движении (покачивание кузова на рессорах).
//
//      Дополняет реакцию камеры от физики (cam_motion): та даёт отклик
//      головы на ускорения (толчки, торможение), раскачка - непрерывное
//      скорость-зависимое покачивание на ходу: подпрыгивание и
//      галопирование (вертикаль), виляние (поперёк), покачивание и
//      клевки (крен/тангаж кузова). Чисто клиентская (визуальная):
//      фаза компонентов - по пройденному пути (f = |v|/lambda, все
//      частоты < 2 Гц - без алиасинга на сетевой интерполяции),
//      амплитуда раскручивается от SwayMinSpeed и достигает полной
//      к SwayRefSpeed; на стоянке - полный покой. Параметры - из
//      settings.xml (секция Viewer, ключи CabineSway*)
//
//------------------------------------------------------------------------------
class CabSway final
{
public:

    CabSway() = default;

    /// Параметры раскачки (из settings_t)
    struct Params
    {
        bool   enabled = true;
        double ref_speed = 25.0;    ///< Скорость полной амплитуды, м/с
        double min_speed = 0.3;     ///< Ниже - покой (плавный вход), м/с
        double max_offset = 0.025;  ///< Лимит смещения от раскачки, м

        /// Амплитуды при ref_speed: м - смещения, рад - углы
        double bounce_amp = 0.006;  ///< Подпрыгивание (вертикаль)
        double gallop_amp = 0.008;  ///< Галопирование (вертикаль)
        double hunt_amp = 0.005;    ///< Виляние (поперёк)
        double roll_amp = 0.005;    ///< Покачивание (крен)
        double pitch_amp = 0.003;   ///< Клевки (тангаж)
    };

    void setParams(const Params& params);

    /// Смена ПЕ: фазовые сдвиги компонентов из адреса ПЕ - соседние
    /// вагоны не качаются синхронно (стабильно в рамках сессии)
    void setVehicle(const void* vehicle);

    /// Шаг: dt - кадр, speed - модуль скорости ПЕ, м/с
    void step(double dt, double speed);

    /// Смещение камеры в локальных осях ПЕ, м: x - поперечное,
    /// y - продольное, z - вертикальное
    vsg::dvec3 offset() const;

    /// Углы раскачки, рад
    double roll() const;
    double pitch() const;

private:

    /// Длины волн компонентов, м (частота = |v|/lambda): короткие -
    /// подпрыгивание/клевки, длинные - галопирование/виляние
    static constexpr double BOUNCE_WAVELENGTH = 17.0;
    static constexpr double GALLOP_WAVELENGTH = 47.0;
    static constexpr double HUNT_WAVELENGTH = 41.0;
    static constexpr double ROLL_WAVELENGTH = 28.0;
    static constexpr double PITCH_WAVELENGTH = 23.0;

    Params _params;

    /// Пройденный путь - фаза раскачки, м
    double _path_s = 0.0;

    /// Фазовые сдвиги компонентов (по ПЕ), рад
    double _phase_bounce = 0.0;
    double _phase_gallop = 0.0;
    double _phase_hunt = 0.0;
    double _phase_roll = 0.0;
    double _phase_pitch = 0.0;

    // Текущие добавки: смещения, м (поперёк/продольное/вертикаль), углы, рад
    double _offset_x = 0.0;
    double _offset_y = 0.0;
    double _offset_z = 0.0;
    double _roll = 0.0;
    double _pitch = 0.0;
};

#endif // CAB_SWAY_H
