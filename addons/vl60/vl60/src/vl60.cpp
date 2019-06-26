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
  , gv_return(0.0)
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
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::step(double t, double dt)
{
    stepPantographsControl(t, dt);

    stepSignalsOutput();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepPantographsControl(double t, double dt)
{
    for (auto pant : pantographs)
    {
        pant->setUks(WIRE_VOLTAGE);
        pant->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VL60::stepSignalsOutput()
{
    // Состояние токоприемников
    analogSignal[40] = static_cast<float>(pantographs[0]->getHeight());
    analogSignal[41] = static_cast<float>(pantographs[1]->getHeight());

    // Состояние тумблеров на пульте машиниста
    analogSignal[TUMBLER_PNT] = static_cast<float>(pants_tumbler.getState());
    analogSignal[TUMBLER_PNT1] = static_cast<float>(pant1_tumbler.getState());
    analogSignal[TUMBLER_PNT2] = static_cast<float>(pant2_tumbler.getState());

    analogSignal[35] = gv_return;

    // Вольтметр КС
    analogSignal[37] = 0.0;

    // Состояние главного выключателя
    analogSignal[42] = gv_pos;

    // Состояние локомотивного светофора
    analogSignal[60] = 1.0f;

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
        // Переводим тумблер внужное фиксированное положени
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
        // Переводим тумблер внужное фиксированное положени
        if (isShift())
            pant2_tumbler.set();
        else
            pant2_tumbler.reset();

        // Задаем статус токоприемнику
        pantographs[1]->setState(isShift() && pants_tumbler.getState());
    }

    if (getKeyState(KEY_P))
    {
        if (isShift())
            gv_pos = 1.0f;
        else
            gv_pos = 0.0f;
    }

    if (getKeyState(KEY_K))
    {
        gv_return = 1.0f;
    }
    else
    {
        gv_return = 0.0f;
    }
}

//------------------------------------------------------------------------------
//  Макрос генерации функции loadVehicle() для симулятора
//------------------------------------------------------------------------------
GET_VEHICLE(VL60)
