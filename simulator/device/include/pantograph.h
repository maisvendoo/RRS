#ifndef PANTOGRAPH_H
#define PANTOGRAPH_H

#include    "device.h"

//------------------------------------------------------------------------------
// Токоприемник пантографного типа
//------------------------------------------------------------------------------
class DEVICE_EXPORT Pantograph : public Device
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

    /// Установить род тока на входе
    void setCurrentKindIn(int kindCurrent);

    /// Получить высоту
    double getHeight() const;

    /// Получить напряжение на выходе
    double getUout() const;

    /// Получить род на выходе
    int getCurrentKindOut() const;


    bool isUp() const;

    bool isDown() const;

    /// Состояние звука подъёма
    sound_state_t getSoundUp() const;

    /// Состояние звука опускания
    sound_state_t getSoundDown() const;

private:

    /// Напряжение в контактной сети
    double Uks;

    /// Напряжение на выходе токоприемника
    double Uout;
/*
    /// Состояние токоприемника (true - подъем, false - опускание)
    bool ref_state;
*/
    /// Максимальная высота подъема (в относительном выражении)
    double  max_height;

    /// Время подъема/опускания ТП
    double  motion_time;

    /// Входное значение рода тока
    int current_kind_in;

    /// Выходное значение рода тока
    int current_kind_out;

    /// Токоприемник поднят
    bool is_up;

    /// Токоприемник опущен
    bool is_down;

    /// Состояние токоприемника (true - подъем, false - опускание)
    Trigger ref_state;
/*
    /// Состояние звука подъёма
    sound_state_t sound_up;

    /// Состояние звука опускания
    sound_state_t sound_down;
*/
    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);

};

#endif // PANTOGRAPH_H
