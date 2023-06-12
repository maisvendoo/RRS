#include    "filesystem.h"

#include    "chs2t.h"

#include    "Journal.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void CHS2T::initPneumoSupply(QString modules_dir)
{
    Q_UNUSED(modules_dir)

    //Journal::instance()->info("Init air suplly subsystem");

    // Регулятор давления в ГР
    press_reg = new PressureRegulator();
    press_reg->read_config("pressure-regulator");

    // Мотор-компрессор
    for (size_t i = 0; i < motor_compressor.size(); ++i)
    {
        motor_compressor[i] = new DCMotorCompressor();
        motor_compressor[i]->read_config("motor-compressor-dc");
        connect(motor_compressor[i], &DCMotorCompressor::soundPlay, this, &CHS2T::soundPlay);
        connect(motor_compressor[i], &DCMotorCompressor::soundStop, this, &CHS2T::soundStop);
        connect(motor_compressor[i], &DCMotorCompressor::soundSetPitch, this, &CHS2T::soundSetPitch);

        motor_compressor[i]->setSoundName(QString("kompressor%1").arg(i+1));

        mk_switcher[i] = new CHS2TSwitcher(Q_NULLPTR, 0, 4);
        mk_switcher[i]->setSoundName("tumbler");
        connect(mk_switcher[i], &Switcher::soundPlay, this, &CHS2T::soundPlay);
    }

    mk_switcher[0]->setKeyCode(KEY_7);
    mk_switcher[1]->setKeyCode(KEY_8);

    // Главный резервуар
    double volume_main = 1.0;
    main_reservoir = new Reservoir(volume_main);
    main_reservoir->setLeakCoeff(1e-6);

    // Концевые краны питательной магистрали
    anglecock_fl_fwd = new PneumoAngleCock();
    anglecock_fl_fwd->read_config("pneumo-anglecock-FL");

    anglecock_fl_bwd = new PneumoAngleCock();
    anglecock_fl_bwd->read_config("pneumo-anglecock-FL");

    // Рукава питательной магистрали
    hose_fl_fwd = new PneumoHose();
    hose_fl_fwd->read_config("pneumo-hose-FL");
    forward_connectors.push_back(hose_fl_fwd);

    hose_fl_bwd = new PneumoHose();
    hose_fl_bwd->read_config("pneumo-hose-FL");
    backward_connectors.push_back(hose_fl_bwd);
}
