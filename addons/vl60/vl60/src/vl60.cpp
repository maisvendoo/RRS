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

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60::VL60() : Vehicle ()
  , Uks(30000.0f)
  , pant1_pos(0.0)
  , pant2_pos(0.0)
  , gv_pos(0.0)
  , gv_return(false)
  , test_lamp(0.0)
  , sig(1)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VL60::~VL60()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::initialization()
{
    QString pant_cfg_path = config_dir + QDir::separator() + "pantograph.xml";

    for (size_t i = 0; i < NUM_PANTOGRAPHS; ++i)
    {
        pantographs[i] = new Pantograph(pant_cfg_path);
    }

    QString gv_cfg_path = config_dir + QDir::separator() + "main-switch.xml";

    main_switch = new MainSwitch(gv_cfg_path);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::step(double t, double dt)
{
    stepPantographsControl(t, dt);

    stepMainSwitchControl(t, dt);

    stepSignalsOutput();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepPantographsControl(double t, double dt)
{
    for (auto pant : pantographs)
    {
        // Задаем текущее напряжение КС (пока что через константу)
        pant->setUks(WIRE_VOLTAGE);
        // Моделируем работу токоприемников
        pant->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepMainSwitchControl(double t, double dt)
{
    // Подаем на вход напряжение с крышевой шины, на которую включены
    // оба токоприемника
    main_switch->setU_in(max(pantographs[0]->getUout(), pantographs[1]->getUout()));

    // Задаем состояние органов управления ГВ
    main_switch->setState(gv_tumbler.getState());
    main_switch->setReturn(gv_return);

    // Подаем питание на удерживающую катушку ГВ
    main_switch->setHoldingCoilState(getHoldingCoilState());

    // Моделируем работу ГВ
    main_switch->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepSignalsOutput()
{
    // Состояние токоприемников
    analogSignal[PANT1_POS] = static_cast<float>(pantographs[0]->getHeight());
    analogSignal[PANT2_POS] = static_cast<float>(pantographs[1]->getHeight());

    // Состояние тумблеров на пульте машиниста
    analogSignal[TUMBLER_PNT] = static_cast<float>(pants_tumbler.getState());
    analogSignal[TUMBLER_PNT1] = static_cast<float>(pant1_tumbler.getState());
    analogSignal[TUMBLER_PNT2] = static_cast<float>(pant2_tumbler.getState());

    analogSignal[TUMBLER_GV_ON] = static_cast<float>(gv_return);
    analogSignal[TUMBLER_GV_ON_OFF] = static_cast<float>(gv_tumbler.getState());

    // Вольтметр КС
    analogSignal[STRELKA_KV2] = 0.0;

    // Состояние главного выключателя
    analogSignal[GV_POS] = static_cast<float>(main_switch->getKnifePos());

    // Состояние локомотивного светофора
    analogSignal[LS_G] = 1.0f;

    // Состояние контрольных ламп
    analogSignal[SIG_LIGHT_GV] = 1.0f;
    analogSignal[SIG_LIGHT_GU] = 1.0f;
    analogSignal[SIG_LIGHT_FR] = 1.0f;
    analogSignal[SIG_LIGHT_0HP] = 1.0f;
    analogSignal[SIG_LIGHT_TR] = 1.0;
    analogSignal[SIG_LIGHT_VU1] = 1.0;
    analogSignal[SIG_LIGHT_VU2] = 1.0;
    analogSignal[SIG_LIGHT_TD] = 1.0;    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VL60::getHoldingCoilState() const
{
    bool state = false; //getKeyState(KEY_T);

    return state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::keyProcess()
{
    // Управление тумблером "Токоприемники"
    if (getKeyState(KEY_O))
    {
        if (isShift())
            pants_tumbler.set();
        else
            pants_tumbler.reset();
    }

    // Подъем/опускание переднего токоприемника
    if (getKeyState(KEY_U))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())                    
            pant1_tumbler.set();
        else        
            pant1_tumbler.reset();

        // Задаем статус токоприемнику
        pantographs[0]->setState(isShift() && pants_tumbler.getState());
    }

    // Подъем/опускание заднего токоприемника
    if (getKeyState(KEY_I))
    {
        // Переводим тумблер в нужное фиксированное положение
        if (isShift())
            pant2_tumbler.set();
        else
            pant2_tumbler.reset();

        // Задаем статус токоприемнику
        pantographs[1]->setState(isShift() && pants_tumbler.getState());
    }

    // Включение/выключение ГВ
    if (getKeyState(KEY_P))
    {
        if (isShift())
            gv_tumbler.set();
        else
            gv_tumbler.reset();
    }

    gv_return = getKeyState(KEY_K);
}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_VEHICLE(VL60)
