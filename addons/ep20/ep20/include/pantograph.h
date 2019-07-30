#ifndef PANTOGRAPH_H
#define PANTOGRAPH_H

#include    "device.h"

//------------------------------------------------------------------------------
// Токоприемник пантографного типа
//------------------------------------------------------------------------------
class Pantograph : public Device
{

public:

    /// Конструктор
    Pantograph(QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~Pantograph();

    /// Установить состояние
    void setState(bool state);

    /// Установить напряжение в кс
    void setUks(double Uks);

    /// Получить высоту
    double getHeight() const;

    /// Получить напряжение на выходе
    double getUout() const;

private:     

    /// Напряжение в контактной сети
    double Uks;

    /// Напряжение на выходе токоприемника
    double Uout;

    /// Состояние токоприемника (true - подъем, false - опускание)
    bool state;
    bool old_state;

    /// Максимальная высота подъема (в относительном выражении)
    double  max_height;

    /// Время подъема/опускания ТП
    double  motion_time;

    enum KindCurrent
    {
        NOT_SPECIFIED = 0, // Род тока неопредлен
        CURRENT_AC = 1, // Переменный ток
        CURRENT_DC = 2 // Постоянный ток
    };

    double RTout;


    void preStep(state_vector_t &Y, double t);


    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);


    void load_config(CfgReader &cfg);

};

#endif // PANTOGRAPH_H
