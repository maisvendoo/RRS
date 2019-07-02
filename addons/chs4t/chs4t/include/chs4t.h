//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз переменного тока ЧС4т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Авилкин Николай (avilkinnick)
//
//      Дата: 16/06/2019
//
//------------------------------------------------------------------------------
#ifndef     CHS4T_H
#define     CHS4T_H

#include    "vehicle-api.h"
#include    "pantograph.h"
#include    "gv.h"

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

    QString     pantograph_config;
    QString     gv_config;

    std::array<Pantograph *, NUM_PANTOGRAPHS>    pantographs;

    GV *glavV;

    /// Шаг моделирования всех систем локомотива в целом
    void step(double t, double dt);

    /// Загрузка данных из конфигурационных файлов
    void loadConfig(QString cfg_path);

    void keyProcess();
};

#endif // CHS4T
