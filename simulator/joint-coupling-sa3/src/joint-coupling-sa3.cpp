#include    "joint-coupling-sa3.h"

#include    "CfgReader.h"
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointCouplingSA3::JointCouplingSA3() : Joint()
  , is_connected(false)
  , delta(0.011)
  , f(0.6)
  , c(2.0e7)
  , lambda(0.11)
  , ck(1.0e9)
  , T0(5000.0)
  , dx_t0(T0/ck)
{
    devices.resize(NUM_CONNECTORS);
/*
    reg = new Registrator();
    reg->setFileName(QString("JointCoupling%1").arg(reinterpret_cast<quint64>(this),0,16));
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
    Q_UNUSED(dt)
//    msg = QString("%1").arg(t,7,'f',3);

    // Расчёт взаимного расположения и скорости
    double x_fwd = devices[FWD]->getOutputSignal(COUPL_OUTPUT_COORD);
    double x_bwd = devices[BWD]->getOutputSignal(COUPL_OUTPUT_COORD);
    double ds = x_fwd - x_bwd;

    double v_fwd = devices[FWD]->getOutputSignal(COUPL_OUTPUT_VELOCITY);
    double v_bwd = devices[BWD]->getOutputSignal(COUPL_OUTPUT_VELOCITY);
    double dv = v_fwd - v_bwd;

    // Сцепление при сближении
    if (ds < Physics::ZERO)
        is_connected = true;

    // Управление расцеплением
    if (   (devices[FWD]->getOutputSignal(COUPL_OUTPUT_REF_STATE) == -1.0)
        || (devices[BWD]->getOutputSignal(COUPL_OUTPUT_REF_STATE) == -1.0) )
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
        // Зазор в сцепках в данный момент, у СА-3 зазор внутрь от оси автосцепки
        ds_delta = cut(ds, -delta, 0.0);
        // Смещение сцепок и поглощающих аппаратов в данный момент
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
            // Смещение сцепок и поглощающих аппаратов в данный момент
            ds_shift = min((ds + delta), 0.0);
        }
        else
        {
            // Зазор в сцепках в данный момент
            ds_delta = ds;
        }
    }

    // Усилия в сцепках
    devices[FWD]->setInputSignal(COUPL_INPUT_FORCE, force);
    devices[BWD]->setInputSignal(COUPL_INPUT_FORCE, force);

    // Зазор в сцепках в данный момент
    devices[FWD]->setInputSignal(COUPL_INPUT_DELTA, ds_delta);
    devices[BWD]->setInputSignal(COUPL_INPUT_DELTA, ds_delta);

    // Смещение сцепок и поглощающих аппаратов в данный момент
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
double JointCouplingSA3::calc_force(double ds, double dv)
{
    // Вычитание зазора в сцепке, у СА-3 зазор внутрь от оси автосцепки
    double x = dead_zone(ds, -delta, 0.0);

    // Деформация до начала сжатия поглощающего аппарата
    double x_t0 = cut(x, -dx_t0, dx_t0);
    // Усилие от упругости конструкций до начала сжатия поглощающих аппаратов
    double force_t0 = x_t0 * ck;
    // Вычитание деформаций до начала сжатия поглощающих аппаратов
    x = dead_zone(x, -dx_t0, dx_t0);

    // Сжатие поглощающих аппаратов
    double x_c = cut(x, -lambda, lambda);
    // Усилие упругих элементов в поглощающих аппаратах
    double force_c = x_c * c;
    // Сила трения фрикционных элементов в поглощающих аппаратах
    double force_f = Physics::fricForce(abs(x_c) * c * f, dv);
    // Вычитание сжатия поглощающих аппаратов
    x = dead_zone(x, -lambda, lambda);

    // Усилие от упругости конструкций
    double force_ck = x * ck;
/*
    msg += QString("%1;%2;%3;%4;%5;%6;%7;%8;%9;%10")
                   .arg(ds,10,'f',6)
                   .arg(dv,10,'f',6)
                   .arg(x_t0,10,'f',6)
                   .arg(force_t0,10,'f',3)
                   .arg(x_c,10,'f',6)
                   .arg(force_c,10,'f',3)
                   .arg(force_f,10,'f',3)
                   .arg(x,10,'f',6)
                   .arg(force_ck,10,'f',3)
                   .arg(force_t0 + force_c + force_f + force_ck,10,'f',3);
*/
    return force_t0 + force_c + force_f + force_ck;
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
    cfg.getDouble(secName, "ck", ck);
    cfg.getDouble(secName, "T0", T0);
    if (ck > Physics::ZERO)
        dx_t0 = T0 / ck;
}

GET_JOINT(JointCouplingSA3)
