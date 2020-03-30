//------------------------------------------------------------------------------
//
//      Магистральный пассажирский тепловоз ТЭП70.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Роман Бирюков (РомычРЖДУЗ)
//
//      Дата: 12/05/2019
//
//------------------------------------------------------------------------------
#ifndef     TEP70_H
#define     TEP70_H

#include    "vehicle-api.h"
#include    "tep70-headers.h"
#include    "tep70-signals.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь тепловоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TEP70 : public Vehicle
{
public:

    /// Конструктор
    TEP70();

    /// Деструктор
    ~TEP70();

private:

    /// Контроллер машиниста
    ControllerKM2202 *km;

    /// Кнопка "Пуск дизеля"
    bool    button_disel_start;

    /// Кнопка "Отпуск тормозов"
    bool    button_brake_release;

    /// Кнопка "Свисток"
    bool    button_svistok;

    /// Кнопка "Тифон"
    bool    button_tifon;

    /// АЗВ "Управление общее"
    Trigger azv_common_control;

    /// АЗВ "Управление тепловозом"
    Trigger azv_upr_tepl;

    /// АЗВ "Топливный насос"
    Trigger azv_fuel_pump;

    /// АЗВ "ЭДТ"
    Trigger azv_edt_on;

    /// АЗВ "Тормоз питание"
    Trigger azv_edt_power;

    /// АЗВ "ЭПТ"
    Trigger azv_ept_on;

    /// Инициализация всех систем тепловоза
    void initialization();

    /// Инициализация органов управления в кабине
    void initCabineControls();

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Шаг моделирования органов управления в кабине
    void stepCabineControls(double t, double dt);

    void stepSignalsOutput(double t, double dt);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработка клавиш
    void keyProcess();
};

#endif // TEP70_H
