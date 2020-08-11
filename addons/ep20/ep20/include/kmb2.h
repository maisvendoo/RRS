#ifndef KMB2_H
#define KMB2_H

#include    "device.h"
#include    "timer.h"

//------------------------------------------------------------------------------
//Класс контроллера машиниста электронного бесконтактного!
//------------------------------------------------------------------------------
class KMB2 : public Device {

public:

    KMB2(QObject *parent = Q_NULLPTR);

    ~KMB2();

    // Получить направление реверса
    float getReverseDir();

    // Получить положение тяги
    float getTractionPosition();

    // Получить положение скорости
    float getVelocityPosition();

    // Получить уровень тяги
    double getTractionLevel();

    // Получить уровень скорости
    double getVelocityLevel();

    // Получить реверсивное состояние
    float getReverseState();

    // Получить состояние разварота Ключ-карты для анимации
    double getTurn();

    // Получить состояние опускания Ключ-карты для анимации
    double getS3();

private:

    // Таймер уровня тяги
    Timer   *tractionVelocityLevelTimer;

    // Таймер состояния реверса
    Timer   *reverseStateTimer;

    // Реверсивное направление
    int  reverse_dir;

    // Положение тяги
    int   traction_pos;

    // Положение скорости
    int   velocity_pos;

    // Уровень тяги (для таймера)
    double   traction_level;

    // Уровень скорости (для таймера)
    double   velocity_level;

    // Реверсивное состояние (для таймера)
    int   reverse_state;

    //Коэффициент из конфига
    double traction_rate;

    //Коэффициент из конфига
    double velocity_rate;

    // Блок переменных для анимации Ключ-карты
    double T;

    double S1;

    double S2;

    double S3;

    bool old_state_W;
    bool old_state_S;
    //----------------------------------------

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

private slots:

    void levelTimerHandler();

    void reverseTimerHandler();
};

#endif // KMB2_H
