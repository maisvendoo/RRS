#ifndef     COUPLING_H
#define     COUPLING_H

#include    "device.h"

#include    "coupling-data.h"

//------------------------------------------------------------------------------
// Сцепное устройство
//------------------------------------------------------------------------------
class DEVICE_EXPORT Coupling : public Device
{
public:

    /// Конструктор
    Coupling(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~Coupling();

    /// Соединить сцепки
    virtual void couple();

    /// Разъединить сцепки
    virtual void uncouple();

    /// Управление сцепкой, по умолчанию: > 0 - сцепить, < 0 - расцепить
    virtual void setCouplingOperatingState(double state);

    /// Состояние сцепки
    bool isCoupled() const;

    /// Задать координату положения сцепки на треке пути, м
    void setCoord(double value);

    /// Задать скорость движения сцепки на треке пути, м/с
    void setVelocity(double value);

    /// Получить тяговое усилие на сцепке в данный момент, Н
    virtual double getCurrentForce() const;

    /// Получить зазор в сцепках в данный момент, м
    virtual double getCurrentDelta() const;

    /// Получить смещение сцепки и поглощающего аппарата в данный момент, м
    virtual double getCurrentShift() const;

    virtual void step(double t, double dt);

private:

    /// Флаг вызова команд управления сцепкой
    bool is_ref_state_command;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg);

};

/*!
 * \typedef
 * \brief getCoupling() signature
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef Coupling* (*GetCoupling)();

/*!
 * \def
 * \brief Macro for getCoupling() generation
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_COUPLING(ClassName) \
    extern "C" Q_DECL_EXPORT Coupling *getCoupling() \
    {\
        return new (ClassName)(); \
    }

/*!
 * \fn
 * \brief Load Coupling from library
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT Coupling *loadCoupling(QString lib_path);

#endif // COUPLING_H
