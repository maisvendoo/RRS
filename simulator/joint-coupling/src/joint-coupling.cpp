#include    "joint-coupling.h"

#include    "CfgReader.h"
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointCoupling::JointCoupling() : Joint()
  , is_connected(false)
  , delta(0.011)
  , f(0.6)
  , c(2.0e7)
  , lambda(0.11)
  , ck(1.0e8)
{
    devices.resize(NUM_CONNECTORS);
/*
    reg = new Registrator();
    reg->setFileName(QString("JointCoupling%1").arg(reinterpret_cast<quint64>(this),0,16));
    reg->init();
    reg->print("   t   ;   ds     ;    dv    ;    x     ;    xc    ;    xck   ;    fc    ;   fck    ;  fc fck  ");
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointCoupling::~JointCoupling()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointCoupling::step(double t, double dt)
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

    // Управление сцепками
    if (   (devices[FWD]->getOutputSignal(COUPL_OUTPUT_REF_STATE) == 1.0)
        || (devices[BWD]->getOutputSignal(COUPL_OUTPUT_REF_STATE) == 1.0) )
    {
        if (ds < delta)
            is_connected = true;
    }

    if (   (devices[FWD]->getOutputSignal(COUPL_OUTPUT_REF_STATE) == -1.0)
        || (devices[BWD]->getOutputSignal(COUPL_OUTPUT_REF_STATE) == -1.0) )
    {
        is_connected = false;
    }

    if (is_connected)
    {
        devices[FWD]->setInputSignal(COUPL_INPUT_IS_CONNECTED, 1.0);
        devices[BWD]->setInputSignal(COUPL_INPUT_IS_CONNECTED, 1.0);

        // Расчёт усилия в сцепке
        double force = calc_force(ds, dv);

        devices[FWD]->setInputSignal(COUPL_INPUT_FORCE, force);
        devices[BWD]->setInputSignal(COUPL_INPUT_FORCE, force);
    }
    else
    {
        devices[FWD]->setInputSignal(COUPL_INPUT_IS_CONNECTED, 0.0);
        devices[BWD]->setInputSignal(COUPL_INPUT_IS_CONNECTED, 0.0);

        // Расцепленные сцепки работают только на сжатие
        if (ds < 0.0)
        {
            // Расчёт усилия в сцепке
            double force = calc_force(ds, dv);

            devices[FWD]->setInputSignal(COUPL_INPUT_FORCE, force);
            devices[BWD]->setInputSignal(COUPL_INPUT_FORCE, force);
        }
        else
        {
            devices[FWD]->setInputSignal(COUPL_INPUT_FORCE, 0.0);
            devices[BWD]->setInputSignal(COUPL_INPUT_FORCE, 0.0);
        }
    }
 /*
    if (abs(ds) > 0.005)
        reg->print(msg);
*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double JointCoupling::calc_force(double ds, double dv)
{
    // Вычитание зазора в сцепке
    double x = dead_zone(ds, -delta / 2.0, delta / 2.0);
    // Сжатие поглощающих аппаратов
    double x_c = cut(x, -lambda, lambda);
    // Усилие в поглощающих аппаратах
    double force_c = c * x_c + Physics::fricForce(f * c * abs(x_c), dv);
    // Сжатие конструкций за вычетом сжатия поглощающих аппаратов
    double x_ck = dead_zone(x_c, -lambda, lambda);
    // Усилие от упругости конструкций
    double force_ck = ck * x_ck;
/*
    msg += QString("%1;%2;%3;%4;%5;%6;%7;%8")
                   .arg(ds,10,'f',6)
                   .arg(dv,10,'f',6)
                   .arg(x,10,'f',6)
                   .arg(x_c,10,'f',6)
                   .arg(x_ck,10,'f',6)
                   .arg(force_c,10,'f',3)
                   .arg(force_ck,10,'f',3)
                   .arg(force_c + force_ck,10,'f',3);
*/
    return force_c + force_ck;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointCoupling::load_config(CfgReader &cfg)
{
    QString secName = "Joint";
    cfg.getBool(secName, "initConnectionState", is_connected);

    cfg.getDouble(secName, "delta", delta);
    cfg.getDouble(secName, "c", c);
    cfg.getDouble(secName, "lambda", lambda);
    cfg.getDouble(secName, "ck", ck);
}

GET_JOINT(JointCoupling)
