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

#include    "vl60.h"
#include    "filesystem.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60::VL60() : Vehicle ()
  , pant1_pos(0.0)
  , pant2_pos(0.0)
  , gv_pos(0.0)
  , gv_return(false)  
  , charge_press(0.0)
  , ip(2.73)
{
    pants_tumbler.setOnSoundName("K_Tumbler_On");
    pants_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pants_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    pant1_tumbler.setOnSoundName("K_Tumbler_On");
    pant1_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pant1_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    pant2_tumbler.setOnSoundName("K_Tumbler_On");
    pant2_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&pant2_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    gv_tumbler.setOnSoundName("K_Tumbler_On");
    gv_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&gv_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    gv_return_tumbler.setOnSoundName("K_Tumbler_Nofixed_On");
    gv_return_tumbler.setOffSoundName("K_Tumbler_Nofixed_Off");
    connect(&gv_return_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    fr_tumbler.setOnSoundName("K_Tumbler_On");
    fr_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&fr_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    for (size_t i = 0; i < mv_tumblers.size(); ++i)
    {
        mv_tumblers[i].setOnSoundName("K_Tumbler_On");
        mv_tumblers[i].setOffSoundName("K_Tumbler_Off");
        connect(&mv_tumblers[i], &Trigger::soundPlay, this, &VL60::soundPlay);
    }

    mk_tumbler.setOnSoundName("K_Tumbler_On");
    mk_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&mk_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    cu_tumbler.setOnSoundName("K_Tumbler_On");
    cu_tumbler.setOffSoundName("K_Tumbler_Off");
    connect(&cu_tumbler, &Trigger::soundPlay, this, &VL60::soundPlay);

    for (size_t i = 0; i < rb.size(); ++i)
    {
        rb[i].setOnSoundName("RB_Down");
        rb[i].setOffSoundName("RB_Up");
        connect(&rb[i], &Trigger::soundPlay, this, &VL60::soundPlay);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60::~VL60()
{

}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_VEHICLE(VL60)
