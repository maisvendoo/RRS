//------------------------------------------------------------------------------
//
//      Магистральный электровоз переменного тока ВЛ60.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 28/03/2019
//
//------------------------------------------------------------------------------

#include    "vl60pk.h"
#include    "filesystem.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60pk::VL60pk() : Vehicle ()
  , U_bat(55.0)
  , pant1_pos(0.0)
  , pant2_pos(0.0)
  , gv_pos(0.0)
  , gv_return(false)
  , charge_press(0.0)
  , ip(2.73)
  , coupling_module_name("sa3")
  , coupling_config_name("sa3")
  , brake_crane_module_name("krm395")
  , brake_crane_config_name("krm395")
  , loco_crane_module_name("kvt254")
  , loco_crane_config_name("kvt254")
  , airdist_module_name("vr292")
  , airdist_config_name("vr292")
  , electro_airdist_module_name("evr305")
  , electro_airdist_config_name("evr305")
  , reg(nullptr)
  , coupling_fwd(nullptr)
  , coupling_bwd(nullptr)
  , oper_rod_fwd(nullptr)
  , oper_rod_bwd(nullptr)
  , ss_svistok(sound_state_t())
  , ss_tifon(sound_state_t())
{
    pants_tumbler.setOnSoundName("K_Tumbler_On");
    pants_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pants_tumbler, &Trigger::soundPlay, this, &VL60pk::soundPlay);

    pant1_tumbler.setOnSoundName("K_Tumbler_On");
    pant1_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pant1_tumbler, &Trigger::soundPlay, this, &VL60pk::soundPlay);

    pant2_tumbler.setOnSoundName("K_Tumbler_On");
    pant2_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pant2_tumbler, &Trigger::soundPlay, this, &VL60pk::soundPlay);

    gv_tumbler.setOnSoundName("K_Tumbler_On");
    gv_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&gv_tumbler, &Trigger::soundPlay, this, &VL60pk::soundPlay);

    gv_return_tumbler.setOnSoundName("K_Tumbler_Nofixed_On");
    gv_return_tumbler.setOffSoundName("K_Tumbler_Nofixed_Off");
    connect(&gv_return_tumbler, &Trigger::soundPlay, this, &VL60pk::soundPlay);

    fr_tumbler.setOnSoundName("K_Tumbler_On");
    fr_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&fr_tumbler, &Trigger::soundPlay, this, &VL60pk::soundPlay);

    for (size_t i = 0; i < mv_tumblers.size(); ++i)
    {
        mv_tumblers[i].setOnSoundName("K_Tumbler_On");
        mv_tumblers[i].setOffSoundName("K_Tumbler_Off");
        connect(&mv_tumblers[i], &Trigger::soundPlay, this, &VL60pk::soundPlay);
    }

    mk_tumbler.setOnSoundName("K_Tumbler_On");
    mk_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&mk_tumbler, &Trigger::soundPlay, this, &VL60pk::soundPlay);

    cu_tumbler.setOnSoundName("K_Tumbler_On");
    cu_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&cu_tumbler, &Trigger::soundPlay, this, &VL60pk::soundPlay);

    for (size_t i = 0; i < rb.size(); ++i)
    {
        rb[i].setOnSoundName("RB_Down");
        rb[i].setOffSoundName("RB_Up");
        connect(&rb[i], &Trigger::soundPlay, this, &VL60pk::soundPlay);
    }

    epb_switch.setOnSoundName("EPT_On");
    epb_switch.setOffSoundName("EPT_Off");
    connect(&epb_switch, &Trigger::soundPlay, this, &VL60pk::soundPlay);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60pk::~VL60pk()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::initialization()
{
    FileSystem &fs = FileSystem::getInstance();
    QString modules_dir = QString(fs.getModulesDir().c_str());
    QString custom_cfg_dir(fs.getVehiclesDir().c_str());
    custom_cfg_dir += fs.separator() + config_dir;

    Uks = WIRE_VOLTAGE;
    current_kind = 1;

    initCouplings(modules_dir, custom_cfg_dir);

    initPantographs(modules_dir, custom_cfg_dir);

    initHighVoltageScheme(modules_dir, custom_cfg_dir);

    initSupplyMachines(modules_dir, custom_cfg_dir);

    initPneumoSupply(modules_dir, custom_cfg_dir);

    initBrakesControl(modules_dir, custom_cfg_dir);

    initBrakesEquipment(modules_dir, custom_cfg_dir);

    initEPB(modules_dir, custom_cfg_dir);

    initTractionControl(modules_dir, custom_cfg_dir);

    initOtherEquipment(modules_dir, custom_cfg_dir);

    initTriggers();

    initTapSounds();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::preStep(double t)
{
    preStepCouplings(t);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60pk::step(double t, double dt)
{
    stepCouplings(t, dt);

    stepPantographsControl(t, dt);

    stepMainSwitchControl(t, dt);

    stepTracTransformer(t, dt);

    stepPhaseSplitter(t, dt);

    stepMotorFans(t, dt);

    stepPneumoSupply(t, dt);

    stepBrakesControl(t, dt);

    stepBrakesEquipment(t, dt);

    stepEPB(t, dt);

    stepTractionControl(t, dt);

    stepLineContactors(t, dt);

    stepOtherEquipment(t, dt);

    stepTapSound();

    stepSoundSignalsOutput(t, dt);

    stepSignalsOutput();

    debugPrint(t, dt);

    autoStartTimer->step(t, dt);
}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_VEHICLE(VL60pk)
