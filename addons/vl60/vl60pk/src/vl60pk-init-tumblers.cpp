#include    "vl60pk.h"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
void VL60pk::initControl(const QString& modules_dir, const QString& custom_cfg_dir)
{
    (void) modules_dir;
    (void) custom_cfg_dir;

    for (auto cab_idx : {CAB1, CAB2})
    {
        // Триггер тумблера "Радиостанция"
        radio_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Автоматическая подача песка"
        autosand_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Тифон"
        P_tifon_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Свисток"
        P_whistle_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Обогрев кабины"
        P_cab_heat_tumbler[cab_idx].setInitState(false);        

        // Триггер тумблера в резерве
        P_reserv1_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Освещение ходовой"
        P_light_chassis_tumbler[cab_idx].setInitState(false);        

        // Триггер тумблера в резерве
        P_reserv2_tumbler[cab_idx].setInitState(false);

        // Триггер тумблера "Проверка АЛСН"
        P_ALSN_check_tumbler[cab_idx].setInitState(false);        
    }
}
