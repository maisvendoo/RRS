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
    float getTractionLevel();

    // Получить уровень скорости
    float getVelocityLevel();

    // Получить реверсивное состояние
    float getReverseState();


    double getPovorot();

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
    int   traction_level;

    // Уровень скорости (для таймера)
    int   velocity_level;

    // Реверсивное состояние (для таймера)
    int   reverse_state;

    //Коэффициент из конфига
    int traction_rate;

    //Коэффициент из конфига
    int velocity_rate;

    double T;

    double S1;

    double S2;

    double S3;

    bool old_state_W;
    bool old_state_S;

    void preStep(state_vector_t &Y, double t);

    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    void load_config(CfgReader &cfg);

    void stepKeysControl(double t, double dt);

private slots:

    void levelTimerHandler();

    void reverseTimerHandler();
};

#endif // KMB2_H
