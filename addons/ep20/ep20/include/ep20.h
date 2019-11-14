//------------------------------------------------------------------------------
//
//      Двухсистемный пассажирский электровоз с асинхронным тяговым приводом ЭП20.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Балашов Евгений (MrJecson)
//
//      Дата: 1/08/2019
//
//------------------------------------------------------------------------------
#ifndef     EP20_H
#define     EP20_H

#include    "vehicle-api.h"
#include    "pantograph.h"
#include    "mpcs.h"
#include    "pant-description.h"
#include    "current-kind-switch.h"
#include    "protective-device.h"

/*!
 * \class
 * \brief Основной класс, описывающий весь тепловоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EP20 : public Vehicle
{
public:

    /// Конструктор
    EP20();

    /// Деструктор
    ~EP20();

private:

//    /// Выбор кабины
//    int selectedCab;

    /// Микропроцессорная система управления электровозом
    MPCS    *mpcs;

    /// Перекрлючатель рода тока
    CurrentKindSwitch   *kindSwitch;

    /// Аппарат защиты (ГВ)
    ProtectiveDevice    *mainSwitch;

    /// Аппарат защиты (БВ)
    ProtectiveDevice    *fastSwitch;

    /// Входные значения
    mpcs_input_t mpcsInput;

    /// Выходные значения
    mpcs_output_t mpcsOutput;

    /// Массив токоприемников
    std::array<Pantograph *, NUM_PANTOGRAPHS> pantograph;

    /// Инициализация
    void initialization();

    /// Инициализация высоковольтной схемы
    void initHighVoltageScheme();

    /// Инициализация МПСУ
    void initMPCS();

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Шаг моделирования МПСУ
    void stepMPCS(double t, double dt);

    /// Шаг моделирования высоковольтной схемы
    void stepHighVoltageScheme(double t, double dt);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    /// Обработчик клавиш
    void keyProcess();
};

#endif // EP20_H

