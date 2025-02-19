#ifndef     SAFETYDEVICE_H
#define     SAFETYDEVICE_H

#include    "device.h"
#include    "vec3.h"

class SpeedMap;

class DEVICE_EXPORT SafetyDevice : public Device
{

public:
    explicit SafetyDevice(QObject *parent = nullptr);
    virtual ~SafetyDevice();

    virtual bool init();

    void step(double t, double dt) override;

    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override;

    /// Прием кода АЛСН
    virtual void setAlsnCode(int code_alsn) = 0;

    /// Прием состояния РБ
    virtual void setRBstate(bool state) = 0;

    /// Прием состояния РБС
    virtual void setRBSstate(bool state) = 0;

    /// Прием состояния РБП
    virtual void setRBPstate(bool state) = 0;

    /// Прием состояния ключа ЭПК
    virtual void setKeyEPK(bool key_epk) = 0;

    /// Прием состояния срыва ЭПК
    virtual void setFailureEPKState(bool failure_EPK_state) = 0;

    /// Прием состояния нулевой позиция контроллера тяги
    virtual void setTractionIsZero(bool traction_is_zero) = 0;

    /// Вернуть флаг работы устройства ТСКБМ
    virtual bool isWorkingTSKBMDevice() const = 0;

    /// Вернуть состояние цепи удерживающей катушки ЭПК
    virtual bool getEPKPowerState() const = 0;

    /// Получить состояние лампы локомотивного световора
    virtual float getLampState(size_t lamp_idx) const = 0;

    /// Получить активную лампу локомотивного светофора
    virtual float getLampNum() = 0;

    virtual double getCurrentSpeedLimit() const = 0;

    virtual double getNextSpeedLimit() const = 0;

    /// Вернуть флаг работы устройства в режиме ЭК
    virtual bool isModeEK() const = 0;

    virtual bool isDisplayON() const;

    virtual void setVoltage(double U_pow) = 0;

    /// Задать конструкционную скорость
    virtual void setMaxVelocity(double v_max) = 0;

    /// Вернуть скорость поезда в км/ч
    virtual double getVelocityKmh() const = 0;

    /// Вернуть состояние индикации сигнала "проверка бдительности"
    virtual int getStateVigilanseCheck() const = 0;

    /// Вернуть ускорение поезда
    virtual double getAcceleration() const = 0;

    /// Задать координату центра локомотива в пространстве
    virtual void setCoord(dvec3 coord) = 0;

    /// Задать координату
    virtual void setRailCoord(double rail_coord) = 0;

    /// Модуль работы с ограничениями скорости на путевой топологии
    virtual void setSpeedMapModule(SpeedMap *device) = 0;

    /// Задать длину поезда
    virtual void setTrainLength(double train_length) = 0;

    /// Задать направление движения поезда(чётное/нечётное)
    virtual void setTrainDirection(int train_dir) = 0;

    /// Задать состояние реверсора
    virtual void setReversorDirection(int reversor_direction);

    virtual double getLimitDistance() const = 0;

    virtual double getRailCoord() const = 0;

    virtual QString getCurStation() const = 0;

    virtual int getStationIndex() const;

    /// Задать имя цели
    virtual void setNameTarget(QString &name_target) = 0;

    /// Вернуть время по графику
    virtual QString getScheduleTime() const = 0;

    /// Задать диаметр колеса
    virtual void setWheelDiameter(double wheel_diameter) = 0;

    /// Задать угловую скороcть колесной пары поезда
    virtual void setOmega(double omega) = 0;

    /// Задать дистанцию до цели
    virtual void setDistanceTarget(int distance_target) = 0;

    /// Вернуть код информационного сообщения
    virtual int getCodeInfoMsg() const = 0;

    /// Загрузка станций из ЭК
    virtual bool loadStationsMap(const QString &path) = 0;

    enum
    {
        NUM_SOUNDS = 3,
        ON_SOUND = 0,
        BUTTON_SOUND = 1,
        OVER_SPEED = 2
    };
};

typedef SafetyDevice* (*GetSafetyDevice)();

#define GET_SAFETY_DEVICE(ClassName) \
    extern "C" Q_DECL_EXPORT SafetyDevice *getSafetyDevice() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT SafetyDevice *loadSafetyDevice(QString lib_path);

#endif // SAFETYDEVICE_H
