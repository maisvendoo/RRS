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

    enum {
        NUM_SOUNDS = 3,
        CHANGE_SOUND = 0,
        UP_SOUND = 1,
        DOWN_SOUND = 2
    };
    /// Состояние звука
    sound_state_t getSoundState(size_t idx = CHANGE_SOUND) const;

    /// Сигнал состояния звука
    float getSoundSignal(size_t idx = CHANGE_SOUND) const;

private:

    /// Напряжение в контактной сети
    double Uks;

    /// Напряжение на выходе токоприемника
    double Uout;

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

    /// Состояние звуков
    std::array<sound_state_t, NUM_SOUNDS> sounds;

    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);

};

#endif // PANTOGRAPH_H
