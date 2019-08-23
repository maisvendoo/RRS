//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------
#ifndef     CHS4T_H
#define     CHS4T_H

#include    "vehicle-api.h"
#include    "pantograph.h"
#include    "protective-device.h"
#include    "km-21kr2.h"
#include    "stepswitch.h"
#include    "pusk-rez.h"
#include    "engine.h"

enum
{
    NUM_PANTOGRAPHS = 2
};

/*!
 * \class
 * \brief Основной класс, описывающий весь тепловоз
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CHS4T : public Vehicle
{
public:

    /// Конструктор
    CHS4T();

    /// Деструктор
    ~CHS4T();    

private:

    QString     vehicle_path;
    QString     pantograph_config;
    QString     gv_config;
    QString     puskrez_config;

    std::array<Pantograph *, NUM_PANTOGRAPHS>    pantographs;

    ProtectiveDevice *bistV;

    PuskRez *puskRez;

    Engine *engine;

    Km21KR2 *km21KR2;

    StepSwitch *stepSwitch;

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    void keyProcess();
};

#endif // CHS4T
