#include    "joint-coupling-sa3.h"

#include    "CfgReader.h"
#include    "physics.h"

#include    "device.h"
#include    "core/get_module.h"

#include    <algorithm>
#include    <cmath>
#include    <sstream>

namespace
{

//------------------------------------------------------------------------------
/// Разбор табличной характеристики "x1:F1;x2:F2;..." (м -> Н).
/// Точки сортируются по деформации (ТЗ "Упругий стержень", п.5)
//------------------------------------------------------------------------------
bool parseCurve(const QString& text, std::vector<std::pair<double, double>>& curve)
{
    curve.clear();

    const QString cleaned = text.simplified();
    if (cleaned.isEmpty())
        return false;

    const QStringList points = cleaned.split(';', Qt::SkipEmptyParts);

    for (const QString& point : points)
    {
        const QStringList coords = point.split(':');

        if (coords.size() != 2)
            continue;

        bool ok_x = false;
        bool ok_f = false;

        const double x = coords[0].toDouble(&ok_x);
        const double f = coords[1].toDouble(&ok_f);

        if (ok_x && ok_f && x >= 0.0)
            curve.emplace_back(x, f);
    }

    std::sort(curve.begin(), curve.end());

    return !curve.empty();
}

//------------------------------------------------------------------------------
/// Линейная интерполяция с экстраполяцией последним наклоном
//------------------------------------------------------------------------------
double interpCurve(const std::vector<std::pair<double, double>>& curve,
                   double x)
{
    if (curve.empty())
        return 0.0;

    if (curve.size() == 1)
        return curve.front().second;

    if (x <= curve.front().first)
    {
        const double k = (curve[1].second - curve[0].second) /
                std::max(curve[1].first - curve[0].first, 1e-9);
        return curve[0].second + k * (x - curve.front().first);
    }

    for (size_t i = 1; i < curve.size(); ++i)
    {
        if (x <= curve[i].first)
        {
            const double k = (curve[i].second - curve[i-1].second) /
                    std::max(curve[i].first - curve[i-1].first, 1e-9);
            return curve[i-1].second + k * (x - curve[i-1].first);
        }
    }

    const size_t last = curve.size() - 1;
    const double k = (curve[last].second - curve[last-1].second) /
            std::max(curve[last].first - curve[last-1].first, 1e-9);

    return curve[last].second + k * (x - curve[last].first);
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointCouplingSA3::JointCouplingSA3() : Joint()
  , is_connected(false)
  , delta(0.011)
  , f(0.6)
  , c(2.0e7)
  , lambda(0.11)
  , fk(0.1)
  , ck(5.0e8)
  , T0(5000.0)
  , dx_t0(T0/ck)
  , max_tension(2.5e6)
  , max_compression(2.0e6)
  , damage_force(1.6e6)
  , break_energy(300.0e3)
  , damping_tension(0.0)
  , damping_compression(0.0)
  , damage(0.0)
  , broken(false)
  , cur_force(0.0)
  , cur_rel_velocity(0.0)
{
    devices.resize(NUM_CONNECTORS);
/*
    reg = new Registrator();
    reg->setFileName(QString("JointSA3%1").arg(reinterpret_cast<quint64>(this),0,16));
    reg->init();
    reg->print("   t   ;    ds    ;    dv    ;   xt0    ;   ft0    ;    xc    ;    fc    ;    ff    ;   xck    ;   fck    ;  summ f ");
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointCouplingSA3::~JointCouplingSA3()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointCouplingSA3::step(double t, double dt)
{
    Q_UNUSED(t)
//    msg = QString("%1").arg(t,7,'f',3);

    // Расчёт взаимного расположения и скорости
    double x_fwd = devices[FWD]->getOutputSignal(COUPL_OUTPUT_COORD);
    double x_bwd = devices[BWD]->getOutputSignal(COUPL_OUTPUT_COORD);
    double ds = x_fwd - x_bwd;

    double v_fwd = devices[FWD]->getOutputSignal(COUPL_OUTPUT_VELOCITY);
    double v_bwd = devices[BWD]->getOutputSignal(COUPL_OUTPUT_VELOCITY);
    double dv = v_fwd - v_bwd;

    cur_rel_velocity = dv;

    // Разрушенная сцепка не передаёт усилий: ПЕ становятся независимыми
    if (broken)
        is_connected = false;

    // Несовместимая пара не сцепляется вовсе (ТЗ coupling, п.2-3)
    if (coupler_type == 3)
        is_connected = false;

    // Сцепление при сближении (разрушенная сцепка не сцепляется вновь).
    // Ограничения (ТЗ coupling, п.3-4): тип сцепки и скорость соударения.
    // Применяются ТОЛЬКО к моменту начального сцепления: уже соединённая
    // пара не размыкается ударной перегрузкой (|dv| выше предела при
    // экстренном торможении)
    if (!broken && !is_connected && (ds < Physics::ZERO))
    {
        // 0 - СА-3, 1 - винтовая (ползучая скорость), 2 - переходная,
        // 3 - несовместимая пара (ТЗ coupling, п.2-3)
        const bool compatible = (coupler_type != 3);

        const double coupling_speed_limit =
                (coupler_type == 1) ? std::min(max_coupling_speed, 0.3)
                                    : max_coupling_speed;

        const bool speed_ok = std::abs(dv) < coupling_speed_limit;

        is_connected = compatible && speed_ok;
    }

    // Управление расцеплением
    if ( (- nf(devices[FWD]->getOutputSignal(COUPL_OUTPUT_REF_STATE))
          - nf(devices[BWD]->getOutputSignal(COUPL_OUTPUT_REF_STATE))
          + 1.0 ) <= Physics::ZERO )
    {
        is_connected = false;
    }

    // Усилия в сцепке
    double force = 0.0;
    // Зазор в сцепках
    double ds_delta = 0.0;
    // Смещение сцепок и поглощающих аппаратов
    double ds_shift = 0.0;

    if (is_connected)
    {
        devices[FWD]->setInputSignal(COUPL_INPUT_IS_CONNECTED, 1.0);
        devices[BWD]->setInputSignal(COUPL_INPUT_IS_CONNECTED, 1.0);

        // Расчёт усилия в сцепке
        force = calc_force(ds, dv);

        // Накопление повреждений и разрушение при перегрузке
        updateDamage(force, dv, dt);

        // Зазор в сцепках в данный момент, у СА-3 зазор внутрь от оси автосцепки
        ds_delta = std::clamp(ds, -delta, 0.0);
        // Смещение сцепок и поглощающих аппаратов
        ds_shift = dead_zone(ds, -delta, 0.0);
    }
    else
    {
        devices[FWD]->setInputSignal(COUPL_INPUT_IS_CONNECTED, 0.0);
        devices[BWD]->setInputSignal(COUPL_INPUT_IS_CONNECTED, 0.0);

        // Расцепленные сцепки работают только на сжатие
        if (ds < 0.0)
        {
            // Расчёт усилия в сцепке
            force = calc_force(ds, dv);
            // Зазор в сцепках в данный момент, у СА-3 зазор внутрь от оси автосцепки
            ds_delta = max(ds, (-delta));
            // Смещение сцепок и поглощающих аппаратов
            ds_shift = min((ds + delta), 0.0);
        }
        else
        {
            // Зазор в сцепках в данный момент
            ds_delta = ds;
        }
    }

    cur_force = force;

    // Усилия в сцепках
    devices[FWD]->setInputSignal(COUPL_INPUT_FORCE, force);
    devices[BWD]->setInputSignal(COUPL_INPUT_FORCE, force);

    // Зазор в сцепках в данный момент
    devices[FWD]->setInputSignal(COUPL_INPUT_DELTA, ds_delta);
    devices[BWD]->setInputSignal(COUPL_INPUT_DELTA, ds_delta);

    // Смещение сцепок и поглощающих аппаратов
    devices[FWD]->setInputSignal(COUPL_INPUT_SHIFT, ds_shift / 2.0);
    devices[BWD]->setInputSignal(COUPL_INPUT_SHIFT, ds_shift / 2.0);
/*
    if ( (ds < -0.011) || (ds > 0.0) )
        reg->print(msg);
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool JointCouplingSA3::isConnected() const
{
    return is_connected;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool JointCouplingSA3::isBroken() const
{
    return broken;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double JointCouplingSA3::getForce() const
{
    return cur_force;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double JointCouplingSA3::getRelVelocity() const
{
    return cur_rel_velocity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double JointCouplingSA3::getDamage() const
{
    return damage;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double JointCouplingSA3::calc_force(double ds, double dv)
{
    // Защита от физического взрыва: нечисловые входы не дают усилия
    if (!std::isfinite(ds) || !std::isfinite(dv))
        return 0.0;

    // Вычитание зазора в сцепке, у СА-3 зазор внутрь от оси автосцепки
    double x = dead_zone(ds, -delta, 0.0);

    double force = 0.0;

    if (!tension_curve.empty() || !compression_curve.empty())
    {
        // Табличная нелинейная характеристика: растяжение и сжатие
        // задаются отдельно (ТЗ "Упругий стержень", п.5-7)
        if (x >= 0.0)
            force = tension_curve.empty() ? 0.0 : interpCurve(tension_curve, x);
        else
            force = compression_curve.empty() ? 0.0 :
                    -interpCurve(compression_curve, -x);
    }
    else
    {
        // Деформация до начала сжатия поглощающего аппарата
        double x_t0 = std::clamp(x, -dx_t0, dx_t0);
        // Усилие от упругости конструкций до начала сжатия поглощающих аппаратов
        double force_t0 = x_t0 * ck;
        // Вычитание деформаций до начала сжатия поглощающих аппаратов
        x = dead_zone(x, -dx_t0, dx_t0);

        // Сжатие поглощающих аппаратов
        double x_c = std::clamp(x, -lambda * 2.0, lambda * 2.0);
        // Усилие упругих элементов в поглощающих аппаратах
        double force_c = x_c * c;
        // Сила трения фрикционных элементов в поглощающих аппаратах
        double force_f = Physics::fricForce(abs(x_c) * c * f, dv);
        // Вычитание сжатия поглощающих аппаратов
        x = dead_zone(x, -lambda * 2.0, lambda * 2.0);

        // Усилие от упругости конструкций
        double force_ck = x * ck;
        // Потери на пластические деформации конструкций
        double force_fk = Physics::fricForce(abs(x) * ck * fk, dv);

        force = force_t0 + force_c + force_f + force_ck + force_fk;
    }

    // Вязкое демпфирование, отдельное на растяжение и сжатие (п.9)
    force += ((dv > 0.0) ? damping_tension : damping_compression) * dv;

    // Ограничение физически невозможных усилий (п.27)
    const double limit = std::max(3.0 * std::max(max_tension, max_compression),
                                  100.0e6);

    return std::clamp(force, -limit, limit);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointCouplingSA3::updateDamage(double force, double dv, double dt)
{
    const double abs_force = abs(force);

    // Предел прочности по знаку нагрузки: растяжение или сжатие
    const double limit = std::max( (force >= 0.0) ? max_tension : max_compression,
                                   1.0);

    // Мгновенное разрушение только при катастрофическом превышении
    // предела прочности (двойное превышение). Обычная перегрузка выше
    // предела не рвёт сцепку мгновенно (ТЗ "Продольная динамика", п.18)
    if (abs_force > 2.0 * limit)
    {
        damage = 1.0;
    }
    else if (abs_force > limit)
    {
        // Быстрое накопление повреждений при сверхпредельной нагрузке:
        // темп растёт с относительным превышением предела
        damage += (abs_force / limit - 1.0) * dt * 10.0;
    }
    else if (abs_force > damage_force)
    {
        // Накопление усталости: работа повреждающей части силы относительно
        // сцепок. Относительная скорость ограничена снизу - статическая
        // перегрузка (длительное растяжение на подъёме) тоже изнашивает
        // сцепку (ТЗ, п.18: не разрушать сразу)
        const double work_rate = (abs_force - damage_force) *
                std::max(abs(dv), 0.05);

        damage += work_rate * dt / std::max(break_energy, 1.0);
    }

    if (damage >= 1.0)
    {
        damage = 1.0;
        broken = true;
        is_connected = false;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointCouplingSA3::load_config(CfgReader &cfg)
{
    QString secName = "Joint";
    cfg.getBool(secName, "initConnectionState", is_connected);

    cfg.getDouble(secName, "delta", delta);
    cfg.getDouble(secName, "c", c);
    cfg.getDouble(secName, "lambda", lambda);
    cfg.getDouble(secName, "fk", fk);
    cfg.getDouble(secName, "ck", ck);
    cfg.getDouble(secName, "T0", T0);
    if (ck > Physics::ZERO)
        dx_t0 = T0 / ck;

    // Пределы прочности и износ (ТЗ "Продольная динамика", п.18, 39)
    cfg.getDouble(secName, "MaxTension", max_tension);
    cfg.getDouble(secName, "MaxCompression", max_compression);
    cfg.getDouble(secName, "DamageForce", damage_force);
    cfg.getDouble(secName, "BreakEnergy", break_energy);

    // Табличные нелинейные характеристики и демпфирование
    // (ТЗ "Упругий стержень", п.5-9)
    QString curve_str = "";
    if (cfg.getString(secName, "TensionCurve", curve_str))
        parseCurve(curve_str, tension_curve);
    if (cfg.getString(secName, "CompressionCurve", curve_str))
        parseCurve(curve_str, compression_curve);

    cfg.getDouble(secName, "DampingTension", damping_tension);
    cfg.getDouble(secName, "DampingCompression", damping_compression);

    QString type_str = "";
    if (cfg.getString(secName, "CouplerType", type_str))
    {
        if (type_str == "sa3")
            coupler_type = 0;
        else if (type_str == "screw")
            coupler_type = 1;
        else if (type_str == "transition")
            coupler_type = 2;
        else if (type_str == "incompatible")
            coupler_type = 3;
    }

    cfg.getDouble(secName, "MaxCouplingSpeed", max_coupling_speed);
}

GET_MODULE(JointCouplingSA3)
