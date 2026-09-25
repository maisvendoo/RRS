//------------------------------------------------------------------------------
//
//      ╨Ь╨░╨│╨╕╤Б╤В╤А╨░╨╗╤М╨╜╤Л╨╣ ╤Н╨╗╨╡╨║╤В╤А╨╛╨▓╨╛╨╖ ╨┐╨╡╤А╨╡╨╝╨╡╨╜╨╜╨╛╨│╨╛ ╤В╨╛╨║╨░ ╨Т╨Ы60.
//      ╨Ф╨╛╨┐╨╛╨╗╨╜╨╡╨╜╨╕╨╡ ╨┤╨╗╤П Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          ╨Ф╨╝╨╕╤В╤А╨╕╨╣ ╨Я╤А╨╕╤В╤Л╨║╨╕╨╜ (maisvendoo),
//          ╨а╨╛╨╝╨░╨╜ ╨С╨╕╤А╤О╨║╨╛╨▓ (╨а╨╛╨╝╤Л╤З╨а╨Ц╨Ф╨г╨Ч)
//
//      ╨Ф╨░╤В╨░: 28/03/2019
//
//------------------------------------------------------------------------------
#ifndef     VL60PK_H
#define     VL60PK_H

//#include "shield-223.h"
//#include "shield-225.h"
//#include "shield-229.h"
#include "vehicle.h"

#include <QString>

#include <QMap>
#include <array>

#include <vl60-autopilot-types.h>

class ACMotorCompressor;
class ACMotorFan;
class AirDistributor;
class AutoTrainStop;
class BrakeCrane;
class PneumoBrakeLock;
class BrakeMech;
class CoilALSN;
class ControllerKME_60_044;
class Coupling;
class DCMotor;
class DecoderALSN;
class EKG_8G;
class ElectroAirDistributor;
class EPBControl;
class EPBConverter;
class LocoCrane;
class OperatingRod;
class Oscillator;
class OverloadRelay;
class Pantograph;
class PhaseSplitter;
class PneumoAngleCock;
class PneumoHose;
class PneumoHoseEPB;
class PneumoRelay;
class PressureRegulator;
class ProtectiveDevice;
class Rectifier;
class Registrator;
class Relay;
class Reservoir;
class SafetyDevice;
class SandingSystem;
class SL2M;
class SpeedMap;
class SpotLight;
class SwitchingValve;
class Timer;
class TracTransformer;
class TrainHorn;

/*!
 * \class
 * \brief ╨Ю╤Б╨╜╨╛╨▓╨╜╨╛╨╣ ╨║╨╗╨░╤Б╤Б, ╨╛╨┐╨╕╤Б╤Л╨▓╨░╤О╤Й╨╕╨╣ ╨▓╨╡╤Б╤М ╤Н╨╗╨╡╨║╤В╤А╨╛╨▓╨╛╨╖
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VL60pk : public Vehicle
{
public:

    /// ╨Ъ╨╛╨╜╤Б╤В╤А╤Г╨║╤В╨╛╤А
    VL60pk();

    /// ╨Ф╨╡╤Б╤В╤А╤Г╨║╤В╨╛╤А
    ~VL60pk();

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╤В╨╛╤А╨╝╨╛╨╖╨╜╤Л╤Е ╨┐╤А╨╕╨▒╨╛╤А╨╛╨▓
    void initBrakeDevices(double p0, double pBP, double pFL) override;

    void OnAutopilot() override;

    void OffAutopilot() override;

private:

    enum
    {
        /// ╨Ю╨▒╤К╨╡╨╝ ╨│╨╗╨░╨▓╨╜╨╛╨│╨╛ ╤А╨╡╨╖╨╡╤А╨▓╤Г╨░╤А╨░ (╨У╨а), ╨╗╨╕╤В╤А╨╛╨▓
        MAIN_RESERVOIR_VOLUME = 1200
    };

    /// ╨Э╨░╨┐╤А╤П╨╢╨╡╨╜╨╕╨╡ ╨░╨║╨║╤Г╨╝╤Г╨╗╤П╤В╨╛╤А╨╜╨╛╨╣ ╨▒╨░╤В╨░╤А╨╡╨╕
    double  U_bat = 55.0;

    float   pant1_pos = 0.0;
    float   pant2_pos = 0.0;
    float   gv_pos = 0.0;
    bool    gv_return = false;

    /// ╨Ч╨░╤А╤П╨┤╨╜╨╛╨╡ ╨┤╨░╨▓╨╗╨╡╨╜╨╕╨╡
    double  charge_press = 0.0;

    /// ╨Я╨╡╤А╨╡╨┤╨░╤В╨╛╤З╨╜╨╛╨╡ ╤З╨╕╤Б╨╗╨╛ ╤А╨╡╨┤╤Г╨║╤В╨╛╤А╨░
    double  ip = 2.73;

    enum
    {
        CABS_NUM = 2,
        CAB1 = 0,
        CAB2 = 1
    };

    enum
    {
        NUM_MOTOR_FANS = 6,
        MV1 = 0,
        MV2 = 1,
        MV3 = 2,
        MV4 = 3,
        MV5 = 4,
        MV6 = 5
    };

    /// ╨Ш╨╝╤П ╨╝╨╛╨┤╤Г╨╗╤П ╤Б╤Ж╨╡╨┐╨╜╨╛╨│╨╛ ╤Г╤Б╤В╤А╨╛╨╣╤Б╤В╨▓╨░
    QString coupling_module_name = "sa3";
    /// ╨Ш╨╝╤П ╨║╨╛╨╜╤Д╨╕╨│╨░ ╤Б╤Ж╨╡╨┐╨╜╨╛╨│╨╛ ╤Г╤Б╤В╤А╨╛╨╣╤Б╤В╨▓╨░
    QString coupling_config_name = "sa3";
    /// ╨Ш╨╝╤П ╨╝╨╛╨┤╤Г╨╗╤П ╨┐╨╛╨╡╨╖╨┤╨╜╨╛╨│╨╛ ╨║╤А╨░╨╜╨░
    QString brake_crane_module_name = "krm395";
    /// ╨Ш╨╝╤П ╨║╨╛╨╜╤Д╨╕╨│╨░ ╨┐╨╛╨╡╨╖╨┤╨╜╨╛╨│╨╛ ╨║╤А╨░╨╜╨░
    QString brake_crane_config_name = "krm395";
    /// ╨Ш╨╝╤П ╨╝╨╛╨┤╤Г╨╗╤П ╨╗╨╛╨║╨╛╨╝╨╛╤В╨╕╨▓╨╜╨╛╨│╨╛ ╨║╤А╨░╨╜╨░
    QString loco_crane_module_name = "kvt254";
    /// ╨Ш╨╝╤П ╨║╨╛╨╜╤Д╨╕╨│╨░ ╨╗╨╛╨║╨╛╨╝╨╛╤В╨╕╨▓╨╜╨╛╨│╨╛ ╨║╤А╨░╨╜╨░
    QString loco_crane_config_name = "kvt254";
    /// ╨Ш╨╝╤П ╨╝╨╛╨┤╤Г╨╗╤П ╨▓╨╛╨╖╨┤╤Г╤Е╨╛╤А╨░╤Б╨┐╤А╨╡╨┤╨╡╨╗╨╕╤В╨╡╨╗╤П
    QString airdist_module_name = "vr292";
    /// ╨Ш╨╝╤П ╨║╨╛╨╜╤Д╨╕╨│╨░ ╨▓╨╛╨╖╨┤╤Г╤Е╨╛╤А╨░╨┐╤А╨╡╨┤╨╡╨╗╨╕╤В╨╡╨╗╤П
    QString airdist_config_name = "vr292";
    /// ╨Ш╨╝╤П ╨╝╨╛╨┤╤Г╨╗╤П ╤Н╨╗╨╡╨║╤В╤А╨╛╨▓╨╛╨╖╨┤╤Г╤Е╨╛╤А╨░╤Б╨┐╤А╨╡╨┤╨╡╨╗╨╕╤В╨╡╨╗╤П
    QString electro_airdist_module_name = "evr305";
    /// ╨Ш╨╝╤П ╨║╨╛╨╜╤Д╨╕╨│╨░ ╤Н╨╗╨╡╨║╤В╤А╨╛╨▓╨╛╨╖╨┤╤Г╤Е╨╛╤А╨░╨┐╤А╨╡╨┤╨╡╨╗╨╕╤В╨╡╨╗╤П
    QString electro_airdist_config_name = "evr305";
    /// ╨Ш╨╝╤П ╨╝╨╛╨┤╤Г╨╗╤П ╨░╨▓╤В╨╛╨▓╨╡╨┤╨╡╨╜╨╕╤П
    QString autopilot_module_name = "vl60-autopilot";
    /// ╨Ш╨╝╤П ╨║╨╛╨╜╤Д╨╕╨│╨░ ╨╝╨╛╨┤╤Г╨╗╤П ╨░╨▓╤В╨╛╨▓╨╡╨┤╨╡╨╜╨╕╤П
    QString autopilot_config_name = "vl60pk-autopilot";
    /// ╨Ъ╨░╤В╨░╨╗╨╛╨│ ╨┐╨╛╨╕╤Б╨║╨░ ╨║╨░╤Б╤В╨╛╨╝╨╜╤Л╤Е ╨╝╨╛╨┤╤Г╨╗╨╡╨╣
    QString custom_modules_dir = "vl60";

    /// ╨а╨╡╨│╨╕╤Б╤В╤А╨░╤В╨╛╤А, ╨┤╨╗╤П ╨╖╨░╨┐╨╕╤Б╨╕ ╨┐╨░╤А╨░╨╝╨╡╤В╤А╨╛╨▓
    Registrator *reg = nullptr;

    /// ╨б╤Ж╨╡╨┐╨║╨░ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    Coupling *coupling_fwd = nullptr;
    /// ╨б╤Ж╨╡╨┐╨║╨░ ╤Б╨╖╨░╨┤╨╕
    Coupling *coupling_bwd = nullptr;

    /// ╨а╨░╤Б╤Ж╨╡╨┐╨╜╨╛╨╣ ╤А╤Л╤З╨░╨│ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    OperatingRod *oper_rod_fwd = nullptr;
    /// ╨а╨░╤Б╤Ж╨╡╨┐╨╜╨╛╨╣ ╤А╤Л╤З╨░╨│ ╤Б╨╖╨░╨┤╨╕
    OperatingRod *oper_rod_bwd = nullptr;

    /// ╨Ф╨░╨╗╤М╨╜╨╕╨╣ ╤А╤П╨┤ ╤В╤Г╨╝╨▒╨╗╨╡╤А╨╛╨▓ ╨┐╤А╨╕╨▒╨╛╤А╨╜╨╛╨╣ ╨┐╨░╨╜╨╡╨╗╨╕ ╨╝╨░╤И╨╕╨╜╨╕╤Б╤В╨░
//    Shield_223 shield223[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨Я╤А╨╛╨╢╨╡╨║╤В╨╛╤А ╤П╤А╨║╨╕╨╣"
    TriggerControl spotlight_high_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨Я╤А╨╛╨╢╨╡╨║╤В╨╛╤А ╤В╤Г╤Б╨║╨╗╤Л╨╣"
    TriggerControl spotlight_low_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨а╨░╨┤╨╕╨╛╤Б╤В╨░╨╜╤Ж╨╕╤П"
    TriggerControl radio_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨ж╨╡╨┐╨╕ ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П"
    TriggerControl cu_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨в╨╛╨║╨╛╨┐╤А╨╕╨╡╨╝╨╜╨╕╨║ ╨┐╨╡╤А╨╡╨┤╨╜╨╕╨╣"
    TriggerControl pant1_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨в╨╛╨║╨╛╨┐╤А╨╕╨╡╨╝╨╜╨╕╨║ ╨╖╨░╨┤╨╜╨╕╨╣"
    TriggerControl pant2_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨в╨╛╨║╨╛╨┐╤А╨╕╨╡╨╝╨╜╨╕╨║╨╕"
    TriggerControl pants_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨У╨Т ╨▓╨║╨╗. ╨Т╨╛╨╖╨▓╤А╨░╤В ╨╖╨░╤Й╨╕╤В╤Л"
    TriggerControl gv_return_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨У╨Т ╨▓╨║╨╗/╨▓╤Л╨║╨╗"
    TriggerControl gv_tumbler[CABS_NUM];

    /// ╨С╨╗╨╕╨╢╨╜╨╕╨╣ ╤А╤П╨┤ ╤В╤Г╨╝╨▒╨╗╨╡╤А╨╛╨▓ ╨┐╤А╨╕╨▒╨╛╤А╨╜╨╛╨╣ ╨┐╨░╨╜╨╡╨╗╨╕ ╨╝╨░╤И╨╕╨╜╨╕╤Б╤В╨░
//    Shield_225 shield225[CABS_NUM];
    /// ╨в╤А╨╕╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨Р╨▓╤В╨╛╨╝╨░╤В╨╕╤З╨╡╤Б╨║╨░╤П ╨┐╨╛╨┤╨░╤З╨░ ╨┐╨╡╤Б╨║╨░"
    TriggerControl autosand_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А╤Л ╤В╤Г╨╝╨▒╨╗╨╡╤А╨╛╨▓ "╨Т╨╡╨╜╤В╨╕╨╗╤П╤В╨╛╤А 1-6"
    TriggerControl mv_tumblers[CABS_NUM][NUM_MOTOR_FANS];
    /// ╨в╤А╨╕╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨Ъ╨╛╨╝╨┐╤А╨╡╤Б╤Б╨╛╤А"
    TriggerControl mk_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨д╨░╨╖╨╛╤А╨░╤Б╤Й╨╡╨┐╨╕╤В╨╡╨╗╤М"
    TriggerControl fr_tumbler[CABS_NUM];

    /// ╨а╤П╨┤ ╤В╤Г╨╝╨▒╨╗╨╡╤А╨╛╨▓ ╨╜╨░ ╨┐╤А╨╕╨▒╨╛╤А╨╜╨╛╨╣ ╨┐╨░╨╜╨╡╨╗╨╕ ╨┐╨╛╨╝╨╛╤Й╨╜╨╕╨║╨░ ╨╝╨░╤И╨╕╨╜╨╕╤Б╤В╨░
//    Shield_229 shield229[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨в╨╕╤Д╨╛╨╜"
    TriggerControl P_tifon_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨б╨▓╨╕╤Б╤В╨╛╨║"
    TriggerControl P_whistle_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨Ю╨▒╨╛╨│╤А╨╡╨▓ ╨║╨░╨▒╨╕╨╜╤Л"
    TriggerControl P_cab_heat_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨в╤Г╤Б╨║╨╗╨╛╨╡ ╨╛╤Б╨▓╨╡╤Й╨╡╨╜╨╕╨╡ ╨║╨░╨▒╨╕╨╜╤Л"
    TriggerControl P_cab_light_low_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨п╤А╨║╨╛╨╡ ╨╛╤Б╨▓╨╡╤Й╨╡╨╜╨╕╨╡ ╨║╨░╨▒╨╕╨╜╤Л"
    TriggerControl P_cab_light_high_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ ╨▓ ╤А╨╡╨╖╨╡╤А╨▓╨╡
    TriggerControl P_reserv1_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨Ю╤Б╨▓╨╡╤Й╨╡╨╜╨╕╨╡ ╤Е╨╛╨┤╨╛╨▓╨╛╨╣"
    TriggerControl P_light_chassis_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨Ю╤Б╨▓╨╡╤Й╨╡╨╜╨╕╨╡ ╨┐╤А╨╕╨▒╨╛╤А╨╛╨▓"
    TriggerControl P_light_devices_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨д╨╛╨╜╨░╤А╤М ╨╗╨╡╨▓╤Л╨╣ ╨▒╤Г╤Д╨╡╤А╨╜╤Л╨╣"
    TriggerControl P_bufferlight_L_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨д╨╛╨╜╨░╤А╤М ╨┐╤А╨░╨▓╤Л╨╣ ╨▒╤Г╤Д╨╡╤А╨╜╤Л╨╣"
    TriggerControl P_bufferlight_R_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ ╨▓ ╤А╨╡╨╖╨╡╤А╨▓╨╡
    TriggerControl P_reserv2_tumbler[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╤В╤Г╨╝╨▒╨╗╨╡╤А╨░ "╨Я╤А╨╛╨▓╨╡╤А╨║╨░ ╨Р╨Ы╨б╨Э"
    TriggerControl P_ALSN_check_tumbler[CABS_NUM];

    /// ╨в╤А╨╕╨│╨│╨╡╤А ╨┐╨╡╤А╨╡╨║╨╗╤О╤З╨░╤В╨╡╨╗╤П "╨Ы╨╡╨▓╤Л╨╣ ╨▒╤Г╤Д╨╡╤А╨╜╤Л╨╣ ╨▒╨╡╨╗╤Л╨╣/╨║╤А╨░╤Б╨╜╤Л╨╣"
    TriggerControl P_buffercolor_L_toogle[CABS_NUM];
    /// ╨в╤А╨╕╨│╨│╨╡╤А ╨┐╨╡╤А╨╡╨║╨╗╤О╤З╨░╤В╨╡╨╗╤П "╨Я╤А╨░╨▓╤Л╨╣ ╨▒╤Г╤Д╨╡╤А╨╜╤Л╨╣ ╨▒╨╡╨╗╤Л╨╣/╨║╤А╨░╤Б╨╜╤Л╨╣"
    TriggerControl P_buffercolor_R_toogle[CABS_NUM];

    enum
    {
        NUM_RB = 3,
        RBS = 0,
        RB_1 = 1,
        RBP = 2
    };

    /// ╨в╤А╨╕╨│╨│╨╡╤А╤Л ╤А╤Г╨║╨╛╤П╤В╨╛╨║ ╨▒╨┤╨╕╤В╨╡╨╗╤М╨╜╨╛╤Б╤В╨╕
    TriggerControl rb[CABS_NUM][NUM_RB];

    /// ╨в╤Г╨╝╨▒╨╗╨╡╤А ╨▓╨║╨╗╤О╤З╨╡╨╜╨╕╤П ╨н╨Я╨в
    TriggerControl epb_switch[CABS_NUM];

    enum
    {
        NUM_PANTOGRAPHS = 2,
        WIRE_VOLTAGE = 25000
    };

    /// ╨в╨╛╨║╨╛╨┐╤А╨╕╨╡╨╝╨╜╨╕╨║╨╕
    std::array<Pantograph *, NUM_PANTOGRAPHS>   pantographs;

    /// ╨У╨╗╨░╨▓╨╜╤Л╨╣ ╨▓╤Л╨║╨╗╤О╤З╨░╤В╨╡╨╗╤М (╨У╨Т)
    ProtectiveDevice    *main_switch = nullptr;

    /// ╨Ь╨╡╤Е╨░╨╜╨╕╨╖╨╝ ╨║╨╕╨╗╨╛╨▓╨╛╨╗╤М╤В╨╝╨╡╤В╤А╨░ ╨Ъ╨б
    Oscillator      *gauge_KV_ks = nullptr;

    /// ╨в╤П╨│╨╛╨▓╤Л╨╣ ╤В╤А╨░╨╜╤Б╤Д╨╛╤А╨╝╨░╤В╨╛╤А
    TracTransformer *trac_trans = nullptr;

    /// ╨Р╤Б╨╕╨╜╤Е╤А╨╛╨╜╨╜╤Л╨╣ ╤А╨░╤Б╤Й╨╡╨┐╨╕╤В╨╡╨╗╤М ╤Д╨░╨╖
    PhaseSplitter   *phase_spliter = nullptr;

    /// ╨Ь╨╛╤В╨╛╤А-╨▓╨╡╨╜╤В╨╕╨╗╤П╤В╨╛╤А╤Л
    std::array<ACMotorFan *, NUM_MOTOR_FANS> motor_fans;

    /// ╨Ь╨╛╤В╨╛╤А-╨║╨╛╨╝╨┐╤А╨╡╤Б╤Б╨╛╤А
    ACMotorCompressor *motor_compressor = nullptr;

    /// ╨а╨╡╨│╤Г╨╗╤П╤В╨╛╤А ╨┤╨░╨▓╨╗╨╡╨╜╨╕╤П ╨▓ ╨У╨а
    PressureRegulator *press_reg = nullptr;

    /// ╨У╨╗╨░╨▓╨╜╤Л╨╣ ╤А╨╡╨╖╨╡╤А╨▓╤Г╨░╤А
    Reservoir   *main_reservoir = nullptr;

    /// ╨Ъ╨╛╨╜╤Ж╨╡╨▓╨╛╨╣ ╨║╤А╨░╨╜ ╨┐╨╕╤В╨░╤В╨╡╨╗╤М╨╜╨╛╨╣ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    PneumoAngleCock *anglecock_fl_fwd = nullptr;

    /// ╨Ъ╨╛╨╜╤Ж╨╡╨▓╨╛╨╣ ╨║╤А╨░╨╜ ╨┐╨╕╤В╨░╤В╨╡╨╗╤М╨╜╨╛╨╣ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤Б╨╖╨░╨┤╨╕
    PneumoAngleCock *anglecock_fl_bwd = nullptr;

    /// ╨а╤Г╨║╨░╨▓ ╨┐╨╕╤В╨░╤В╨╡╨╗╤М╨╜╨╛╨╣  ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    PneumoHose      *hose_fl_fwd = nullptr;

    /// ╨а╤Г╨║╨░╨▓ ╨┐╨╕╤В╨░╤В╨╡╨╗╤М╨╜╨╛╨╣  ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤Б╨╖╨░╨┤╨╕
    PneumoHose      *hose_fl_bwd = nullptr;

    /// ╨С╨╗╨╛╨║╨╕╤А╨╛╨▓╨╛╤З╨╜╨╛╨╡ ╤Г╤Б╤В╤А╨╛╨╣╤Б╤В╨▓╨╛ ╨г╨С╨в ╤Г╤Б╨╗.тДЦ367╨╝
    PneumoBrakeLock *brake_lock[CABS_NUM] = {nullptr, nullptr};

    /// ╨Я╨╛╨╡╨╖╨┤╨╜╨╛╨╣ ╨║╤А╨░╨╜ ╨╝╨░╤И╨╕╨╜╨╕╤Б╤В╨░ ╤Г╤Б╨╗.тДЦ395
    BrakeCrane  *brake_crane[CABS_NUM] = {nullptr, nullptr};

    /// ╨Ъ╤А╨░╨╜ ╨▓╨┐╨╛╨╝╨╛╨│╨░╤В╨╡╨╗╤М╨╜╨╛╨│╨╛ ╤В╨╛╤А╨╝╨╛╨╖╨░ ╤Г╤Б╨╗.тДЦ254
    LocoCrane   *loco_crane[CABS_NUM] = {nullptr, nullptr};

    /// ╨в╨╛╤А╨╝╨╛╨╖╨╜╨░╤П ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╤М
    Reservoir   *brakepipe = nullptr;

    /// ╨Т╨╛╨╖╨┤╤Г╤Е╨╛╤А╨░╤Б╨┐╤А╨╡╨┤╨╡╨╗╨╕╤В╨╡╨╗╤М
    AirDistributor  *air_dist = nullptr;

    /// ╨н╨╗╨╡╨║╤В╤А╨╛╨▓╨╛╨╖╨┤╤Г╤Е╨╛╤А╨░╤Б╨┐╤А╨╡╨┤╨╡╨╗╨╕╤В╨╡╨╗╤М
    ElectroAirDistributor  *electro_air_dist = nullptr;

    /// ╨Ч╨░╨┐╨░╤Б╨╜╤Л╨╣ ╤А╨╡╨╖╨╡╤А╨▓╤Г╨░╤А
    Reservoir   *supply_reservoir = nullptr;

    /// ╨Ъ╨╛╨╜╤Ж╨╡╨▓╨╛╨╣ ╨║╤А╨░╨╜ ╤В╨╛╤А╨╝╨╛╨╖╨╜╨╛╨╣ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    PneumoAngleCock *anglecock_bp_fwd = nullptr;

    /// ╨Ъ╨╛╨╜╤Ж╨╡╨▓╨╛╨╣ ╨║╤А╨░╨╜ ╤В╨╛╤А╨╝╨╛╨╖╨╜╨╛╨╣ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤Б╨╖╨░╨┤╨╕
    PneumoAngleCock *anglecock_bp_bwd = nullptr;

    /// ╨а╤Г╨║╨░╨▓ ╤В╨╛╤А╨╝╨╛╨╖╨╜╨╛╨╣ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    PneumoHoseEPB   *hose_bp_fwd = nullptr;

    /// ╨а╤Г╨║╨░╨▓ ╤В╨╛╤А╨╝╨╛╨╖╨╜╨╛╨╣ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤Б╨╖╨░╨┤╨╕
    PneumoHoseEPB   *hose_bp_bwd = nullptr;

    /// ╨Я╨╡╤А╨╡╨║╨╗╤О╤З╨░╤В╨╡╨╗╤М╨╜╤Л╨╣ ╨║╨╗╨░╨┐╨░╨╜ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤В╨╛╤А╨╝╨╛╨╖╨╜╤Л╤Е ╤Ж╨╕╨╗╨╕╨╜╨┤╤А╨╛╨▓ ╨Ч╨Я╨Ъ
    SwitchingValve  *bc_switch_valve = nullptr;

    /// ╨Я╨╛╨▓╤В╨╛╤А╨╕╤В╨╡╨╗╤М╨╜╨╛╨╡ ╤А╨╡╨╗╨╡ ╨┤╨░╨▓╨╗╨╡╨╜╨╕╤П ╤Г╤Б╨╗.тДЦ304
    PneumoRelay     *bc_pressure_relay = nullptr;

    enum
    {
        NUM_TROLLEYS = 2,
        NUM_AXIS_PER_TROLLEY = 3,
        TROLLEY_FWD = 0,
        TROLLEY_BWD = 1
    };

    /// ╨в╨╛╤А╨╝╨╛╨╖╨╜╤Л╨╡ ╨╝╨╡╤Е╨░╨╜╨╕╨╖╨╝╤Л ╤В╨╡╨╗╨╡╨╢╨╡╨║
    std::array<BrakeMech *, NUM_TROLLEYS> brake_mech;

    /// ╨Ъ╨╛╨╜╤Ж╨╡╨▓╨╛╨╣ ╨║╤А╨░╨╜ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤В╨╛╤А╨╝╨╛╨╖╨╜╤Л╤Е ╤Ж╨╕╨╗╨╕╨╜╨┤╤А╨╛╨▓ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    PneumoAngleCock  *anglecock_bc_fwd = nullptr;

    /// ╨Ъ╨╛╨╜╤Ж╨╡╨▓╨╛╨╣ ╨║╤А╨░╨╜ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤В╨╛╤А╨╝╨╛╨╖╨╜╤Л╤Е ╤Ж╨╕╨╗╨╕╨╜╨┤╤А╨╛╨▓ ╤Б╨╖╨░╨┤╨╕
    PneumoAngleCock  *anglecock_bc_bwd = nullptr;

    /// ╨а╤Г╨║╨░╨▓ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤В╨╛╤А╨╝╨╛╨╖╨╜╤Л╤Е ╤Ж╨╕╨╗╨╕╨╜╨┤╤А╨╛╨▓ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    PneumoHose  *hose_bc_fwd = nullptr;

    /// ╨а╤Г╨║╨░╨▓ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕ ╤В╨╛╤А╨╝╨╛╨╖╨╜╤Л╤Е ╤Ж╨╕╨╗╨╕╨╜╨┤╤А╨╛╨▓ ╤Б╨╖╨░╨┤╨╕
    PneumoHose  *hose_bc_bwd = nullptr;

    /// ╨Ш╤Б╤В╨╛╤З╨╜╨╕╨║ ╨┐╨╕╤В╨░╨╜╨╕╤П ╨н╨Я╨в
    EPBConverter    *epb_converter = nullptr;

    /// ╨С╨╗╨╛╨║ ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П ╨┤╨▓╤Г╤Е╨┐╤А╨╛╨▓╨╛╨┤╨╜╨╛╨│╨╛ ╨н╨Я╨в
    EPBControl  *epb_control = nullptr;

    /// ╨Ъ╨╛╨╜╤В╤А╨╛╨╗╨╗╨╡╤А ╨╝╨░╤И╨╕╨╜╨╕╤Б╤В╨░
    ControllerKME_60_044    *controller[CABS_NUM] = {nullptr, nullptr};

    /// ╨У╨╗╨░╨▓╨╜╤Л╨╣ ╨║╨╛╨╜╤В╤А╨╛╨╗╨╗╨╡╤А (╨┐╨╡╤А╨╡╨║╨╗╤О╤З╨╡╨╜╨╕╨╡ ╨╛╨▒╨╝╨╛╤В╨╛╨║ ╤В╤П╨│╨╛╨▓╨╛╨│╨╛ ╤В╤А╨░╨╜╤Б╤Д╨╛╤А╨╝╨░╤В╨╛╤А╨░)
    EKG_8G                  *main_controller = nullptr;

    enum
    {
        NUM_VU = 2,
        VU1 = 0,
        VU2 = 1
    };

    /// ╨Т╤Л╨┐╤А╤П╨╝╨╕╤В╨╡╨╗╤М╨╜╤Л╨╡ ╤Г╤Б╤В╨░╨╜╨╛╨▓╨║╨╕
    std::array<Rectifier *, NUM_VU> vu;

    /// ╨Ь╨╡╤Е╨░╨╜╨╕╨╖╨╝ ╨║╨╕╨╗╨╛╨▓╨╛╨╗╤М╤В╨╝╨╡╤В╤А╨░ ╨в╨н╨Ф
    Oscillator  *gauge_KV_motors = nullptr;

    enum
    {
        NUM_MOTORS = 6,
        TED1 = 0,
        TED2 = 1,
        TED3 = 2,
        TED4 = 3,
        TED5 = 4,
        TED6 = 5
    };

    /// ╨в╤П╨│╨╛╨▓╤Л╨╡ ╤Н╨╗╨╡╨║╤В╤А╨╛╨┤╨▓╨╕╨│╨░╤В╨╡╨╗╨╕
    std::array<DCMotor *, NUM_MOTORS>  motor;

    /// ╨а╨╡╨╗╨╡ ╨┐╨╡╤А╨╡╨│╤А╤Г╨╖╨║╨╕ ╨в╨н╨Ф
    std::array<OverloadRelay *, NUM_MOTORS> overload_relay;

    /// ╨Ы╨╕╨╜╨╡╨╣╨╜╤Л╨╡ ╨║╨╛╨╜╤В╨░╨║╤В╨╛╤А╤Л ╨в╨н╨Ф
    std::array<Trigger, NUM_MOTORS> line_contactor;

    /// ╨Ю╨│╤А╨░╨╜╨╕╤З╨╡╨╜╨╕╤П ╤Б╨║╨╛╤А╨╛╤Б╤В╨╕ ╨╜╨░ ╨┐╤Г╤В╨╡╨▓╨╛╨╣ ╨╕╨╜╤Д╤А╨░╤Б╤В╤А╤Г╨║╤В╤Г╤А╨╡ ╨┤╨╗╤П ╨║╨░╨▒╨╕╨╜╤Л ╨Р
    SpeedMap    *speedmap_fwd = nullptr;
    /// ╨Ю╨│╤А╨░╨╜╨╕╤З╨╡╨╜╨╕╤П ╤Б╨║╨╛╤А╨╛╤Б╤В╨╕ ╨╜╨░ ╨┐╤Г╤В╨╡╨▓╨╛╨╣ ╨╕╨╜╤Д╤А╨░╤Б╤В╤А╤Г╨║╤В╤Г╤А╨╡ ╨┤╨╗╤П ╨║╨░╨▒╨╕╨╜╤Л ╨С
    SpeedMap    *speedmap_bwd = nullptr;

    /// ╨Я╤А╨╕╤С╨╝╨╜╨░╤П ╨║╨░╤В╤Г╤И╨║╨░ ╨Р╨Ы╨б╨Э ╨┤╨╗╤П ╨║╨░╨▒╨╕╨╜╤Л ╨Р
    CoilALSN    *coil_ALSN_fwd = nullptr;
    /// ╨Я╤А╨╕╤С╨╝╨╜╨░╤П ╨║╨░╤В╤Г╤И╨║╨░ ╨Р╨Ы╨б╨Э ╨┤╨╗╤П ╨║╨░╨▒╨╕╨╜╤Л ╨С
    CoilALSN    *coil_ALSN_bwd = nullptr;

    /// ╨Ы╨╛╨║╨╛╨╝╨╛╤В╨╕╨▓╨╜╤Л╨╣ ╤Б╨║╨╛╤А╨╛╤Б╤В╨╡╨╝╨╡╤А
    SL2M    *speed_meter[CABS_NUM] = {nullptr, nullptr};

    /// ╨б╨▓╨╕╤Б╤В╨╛╨║ ╨╕ ╤В╨╕╤Д╨╛╨╜
    TrainHorn   *horn[CABS_NUM] = {nullptr, nullptr};

    /// ╨б╨╕╤Б╤В╨╡╨╝╨░ ╨┐╨╛╨┤╨░╤З╨╕ ╨┐╨╡╤Б╨║╨░
    SandingSystem   *sand_system = nullptr;

    std::vector<Trigger *> triggers;
    Timer   *autoStartTimer = nullptr;
    size_t  start_count = 0;
    size_t  autostart_cab = 0;

    /// ╨а╨╡╨╢╨╕╨╝ ╨░╨▓╤В╨╛╨╝╨░╤В╨╕╤З╨╡╤Б╨║╨╛╨│╨╛ ╨╖╨░╨┐╤Г╤Б╨║╨░/╨╛╤Б╤В╨░╨╜╨╛╨▓╨░
    enum AutostartMode
    {
        AUTOSTART_IDLE = 0, ///< ╨Э╨╡╤В ╨░╨║╤В╨╕╨▓╨╜╨╛╨╣ ╨┐╤А╨╛╨│╤А╨░╨╝╨╝╤Л
        AUTOSTART_ON   = 1, ///< ╨Т╤Л╨┐╨╛╨╗╨╜╤П╨╡╤В╤Б╤П ╨░╨▓╤В╨╛╨╖╨░╨┐╤Г╤Б╨║
        AUTOSTART_OFF  = 2  ///< ╨Т╤Л╨┐╨╛╨╗╨╜╤П╨╡╤В╤Б╤П ╨░╨▓╤В╨╛╨╛╤Б╤В╨░╨╜╨╛╨▓
    };
    AutostartMode autostart_mode = AUTOSTART_IDLE;

    /// ╨г╤Б╤В╤А╨╛╨╣╤Б╤В╨▓╨╛ ╨▒╨╡╨╖╨╛╨┐╨░╤Б╨╜╨╛╤Б╤В╨╕ ╨г╨Ъ╨С╨Ь
    SafetyDevice *safety_device[CABS_NUM] = {nullptr, nullptr};

    /// ╨н╨╗╨╡╨║╤В╤А╨╛╨┐╨╜╨╡╨▓╨╝╨░╤В╨╕╤З╨╡╤Б╨║╨╕╨╣ ╨║╨╗╨░╨┐╨░╨╜ ╨░╨▓╤В╨╛╤Б╤В╨╛╨┐╨░
    AutoTrainStop *epk[CABS_NUM] = {nullptr, nullptr};

    enum
    {
        NUM_LC_CONTACTS = 3,
        LC_SELF = 0,
        LC_TED = 1,
        LC_TED_LAMP = 2
    };

    /// ╨Ы╨╕╨╜╨╡╨╣╨╜╤Л╨╡ ╨║╨╛╨╜╤В╨░╨║╤В╨╛╤А╤Л ╤В╤П╨│╨╛╨▓╤Л╤Е ╨┤╨▓╨╕╨│╨░╤В╨╡╨╗╨╡╨╣
    std::array<Relay *, NUM_MOTORS> linear_contactor;

    DecoderALSN *alsn_decoder[CABS_NUM] = {nullptr, nullptr};

    SpotLight *spotlight[CABS_NUM] = {Q_NULLPTR, Q_NULLPTR};

    TriggerControl autopilot_switcher[CABS_NUM];

    vl60_control_t *auto_control[CABS_NUM] = {nullptr, nullptr};

    vl60_feedback_t *auto_feedback[CABS_NUM];

    /// ╨в╤Г╨╝╨▒╨╗╨╡╤А "╨Ь╨░╨╜╨╡╨▓╤А╨╛╨▓╤Л╨╣/╨Я╨╛╨╡╨╖╨┤╨╜╨╛╨╣"
    TriggerControl tumbler_shunting_mode[CABS_NUM];

    /// ╨з╤В╨╡╨╜╨╕╨╡ ╨║╨╛╨╜╤Д╨╕╨│╤Г╤А╨░╤Ж╨╕╨╛╨╜╨╜╨╛╨│╨╛ ╤Д╨░╨╣╨╗╨░
    void loadConfig(QString cfg_path) override;


    /// ╨Ю╨▒╤Й╨░╤П ╨╕╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╨╗╨╛╨║╨╛╨╝╨╛╤В╨╕╨▓╨░
    void initialization() override;

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╤Б╤Ж╨╡╨┐╨╜╤Л╤Е ╤Г╤Б╤В╤А╨╛╨╣╤Б╤В╨▓
    void initCouplings(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╤В╨╛╨║╨╛╨┐╤А╨╕╨╡╨╝╨╜╨╕╨║╨╛╨▓
    void initPantographs(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨░╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╨▓╤Л╤Б╨╛╨║╨╛╨▓╨╛╨╗╤М╤В╨╜╨╛╨╣ ╤З╨░╤Б╤В╨╕ ╤Б╤Е╨╡╨╝╤Л (╨У╨Т, ╤В╤П╨│╨╛╨▓╤Л╨╣ ╤В╤А╨░╨╜╤Б╤Д╨╛╤А╨╝╨░╤В╨╛╤А)
    void initHighVoltageScheme(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╨▓╤Б╨┐╨╛╨╝╨╛╨│╨░╤В╨╡╨╗╤М╨╜╤Л╤Е ╨╝╨░╤И╨╕╨╜ (╨д╨а, ╨Ь╨Ъ, ╨Ь╨Т1 - ╨Ь╨Т6)
    void initSupplyMachines(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╨┐╨╕╤В╨░╤В╨╡╨╗╤М╨╜╨╛╨╣ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕
    void initPneumoSupply(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╨┐╤А╨╕╨▒╨╛╤А╨╛╨▓ ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П ╤В╨╛╤А╨╝╨╛╨╖╨░╨╝╨╕
    void initBrakesControl(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╤В╨╛╤А╨╝╨╛╨╖╨╜╨╛╨│╨╛ ╨╛╨▒╨╛╤А╤Г╨┤╨╛╨▓╨░╨╜╨╕╤П
    void initBrakesEquipment(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╨н╨Я╨в
    void initEPB(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╤Б╤Е╨╡╨╝╤Л ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П ╤В╤П╨│╨╛╨╣
    void initTractionControl(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╨┐╤А╨╕╨▒╨╛╤А╨╛╨▓ ╨▒╨╡╨╖╨╛╨┐╨░╤Б╨╜╨╛╤Б╤В╨╕
    void initSafetyDevices(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╨┐╤А╨╛╤З╨╡╨│╨╛ ╨╛╨▒╨╛╤А╤Г╨┤╨╛╨▓╨░╨╜╨╕╤П
    void initOtherEquipment(const QString& modules_dir, const QString& custom_cfg_dir);

    /// ╨Ш╨╜╨╕╤Ж╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П
    void initControl(const QString& modules_dir, const QString& custom_cfg_dir);

    bool initAutostartProgram(int cab_autostart_request);

    void buildAutostartTriggers(int cab);

    bool initAutostopProgram(int cab_autostop_request);

    void initAutopilot(const QString& modules_dir, const QString& custom_cfg_dir);

    void prepareCabineForAutopilot(int my_cab_idx, int other_cab_idx);

    /// ╨Я╤А╨╛╤Ж╨╡╤Б╤Б ╤Б╨╕╨╝╤Г╨╗╤П╤Ж╨╕╨╕
    void process(const simulator_time_t& t, const double& dt) override;

    /// ╨г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╨╡
    void keyProcess(const simulator_time_t& t, const double& dt);

    /// ╨Ю╤В╨╗╨░╨┤╨╛╤З╨╜╨░╤П ╤Б╤В╤А╨╛╨║╨░
    void debugPrint(const simulator_time_t& t, const double& dt);

    /// ╨б╨╕╨│╨╜╨░╨╗╤Л ╨┤╨╗╤П ╨░╨╜╨╕╨╝╨░╤Ж╨╕╨╕
    void signalsOutput(const simulator_time_t& t, const double& dt);

    /// ╨б╨╕╨│╨╜╨░╨╗╤Л ╨┤╨╗╤П ╨╛╨╖╨▓╤Г╤З╨║╨╕
    void soundsOutput(const simulator_time_t& t, const double& dt);


    /// ╨Я╤А╨╡╨┤╨▓╨░╤А╨╕╤В╨╡╨╗╤М╨╜╤Л╨╡ ╤А╨░╤Б╤З╤С╤В╤Л ╨┐╨╡╤А╨╡╨┤ ╤Б╨╕╨╝╤Г╨╗╤П╤Ж╨╕╨╡╨╣
    void preStep(const double& t) override;

    /// ╨Я╤А╨╡╨┤╨▓╨░╤А╨╕╤В╨╡╨╗╤М╨╜╤Л╨╣ ╤А╨░╤Б╤З╤С╤В ╨║╨╛╨╛╤А╨┤╨╕╨╜╨░╤В ╤Б╤Ж╨╡╨┐╨╜╤Л╤Е ╤Г╤Б╤В╤А╨╛╨╣╤Б╤В╨▓
    void preStepCouplings(const double& t);


    /// ╨и╨░╨│ ╤Б╨╕╨╝╤Г╨╗╤П╤Ж╨╕╨╕ ╨▓╤Б╨╡╤Е ╤Б╨╕╤Б╤В╨╡╨╝ ╤Н╨╗╨╡╨║╤В╤А╨╛╨▓╨╛╨╖╨░
    void step(const double& t, const double& dt) override;

    /// ╨Ь╨╛╨┤╨╡╨╗╨╕╤А╨╛╨▓╨░╨╜╨╕╨╡ ╤Б╤Ж╨╡╨┐╨╜╤Л╤Е ╤Г╤Б╤В╤А╨╛╨╣╤Б╤В╨▓
    void stepCouplings(const double& t, const double& dt);

    void stepPantographsControl(const double& t, const double& dt);

    void stepMainSwitchControl(const double& t, const double& dt);

    void stepTracTransformer(const double& t, const double& dt);

    void stepPhaseSplitter(const double& t, const double& dt);

    void stepMotorFans(const double& t, const double& dt);

    /// ╨Ь╨╛╨┤╨╡╨╗╨╕╤А╨╛╨▓╨░╨╜╨╕╨╡ ╨┐╨╕╤В╨░╤В╨╡╨╗╤М╨╜╨╛╨╣ ╨╝╨░╨│╨╕╤Б╤В╤А╨░╨╗╨╕
    void stepPneumoSupply(const double& t, const double& dt);

    /// ╨Ь╨╛╨┤╨╡╨╗╨╕╤А╨╛╨▓╨░╨╜╨╕╨╡ ╨┐╤А╨╕╨▒╨╛╤А╨╛╨▓ ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П ╤В╨╛╤А╨╝╨╛╨╖╨░╨╝╨╕
    void stepBrakesControl(const double& t, const double& dt);

    /// ╨Ь╨╛╨┤╨╡╨╗╨╕╤А╨╛╨▓╨░╨╜╨╕╨╡ ╤В╨╛╤А╨╝╨╛╨╖╨╜╨╛╨│╨╛ ╨╛╨▒╨╛╤А╤Г╨┤╨╛╨▓╨░╨╜╨╕╤П
    void stepBrakesEquipment(const double& t, const double& dt);

    /// ╨Ь╨╛╨┤╨╡╨╗╨╕╤А╨╛╨▓╨░╨╜╨╕╨╡ ╨н╨Я╨в
    void stepEPB(const double& t, const double& dt);

    void stepTractionControl(const double& t, const double& dt);

    void stepLineContactors(const double& t, const double& dt);

    void stepOtherEquipment(const double& t, const double& dt);

    /// ╨Ь╨╛╨┤╨╡╨╗╨╕╤А╨╛╨▓╨░╨╜╨╕╨╡ ╨┐╤А╨╕╨▒╨╛╤А╨╛╨▓ ╨▒╨╡╨╖╨╛╨┐╨░╤Б╨╜╨╛╤Б╤В╨╕
    void stepSafetyDevices(const double& t, const double& dt);

    void stepControls(const double &t, const double &dt);

    /// ╨Я╤А╨╕╨╝╨╡╨╜╨╡╨╜╨╕╨╡ ╨╛╨┤╨╜╨╛╨╣ ╨║╨╛╨╝╨░╨╜╨┤╤Л ╤Г╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╤П ╨║ ╤Г╤Б╤В╤А╨╛╨╣╤Б╤В╨▓╨░╨╝
    void applyControlCommand(int cab_idx, int id, float value);

    /// ╨Я╤А╨╡╨┤╤Л╨┤╤Г╤Й╨╕╨╡ ╨╖╨╜╨░╤З╨╡╨╜╨╕╤П ╨║╨╛╨╝╨░╨╜╨┤ (╨┤╨╡╤В╨╡╨║╤В╨╛╤А ╤Д╤А╨╛╨╜╤В╨░ ╨▓ stepControls)
    std::array<QMap<int, float>, CABS_NUM> prev_control_values;

    /// ╨Р╨▓╤В╨╛╨▓╨╡╨┤╨╡╨╜╨╕╨╡
    void stepAutopilot(double t, double dt);

    void lineContactorsControl(bool state);

    float isLineContactorsOff();

    double getTractionForce();

    bool getHoldingCoilState() const;

    void load_brakes_config(QString path);

private slots:

    void slotAutoStart();

    void slotAutoStop();

    void slotInitTrainForAutopilot();
};

#endif // VL60PK_H

