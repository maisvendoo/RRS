#ifndef     AUTOPILOT_H
#define     AUTOPILOT_H

#include    <device.h>
#include    <autopilot-types.h>

/*!
 * \class
 * \bref Абстрактный интерфейс для реализации функций систем автоведения
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Autopilot : public Device
{
public:

    Autopilot(QObject *parent = nullptr) : Device(parent)
    {
        connect(rb_timer, &Timer::process, this, &Autopilot::slotVigilanceControl);
    }

    ~Autopilot()
    {

    }

    /// Выдать сигналы управления для органов управления ПС
    virtual auto_control_t *getControl() = 0;

    /// Получить обратную связь от систем ПС
    virtual void setFeedback(auto_feedback_t *feedback)
    {
        this->feedback = feedback;
    }

    void on()
    {
        is_active = true;
    }

    void off()
    {
        is_active = false;
    }

    bool isActive() const
    {
        return is_active;
    }

    void step(double t, double dt) override;

    void setActiveCabine(uint8_t cab_idx)
    {
        this->cab_idx = cab_idx;
    }

    uint8_t getActiveCabine() const
    {
        return cab_idx;
    }    

protected:

    /// Признак активации
    bool is_active = false;

    /// Номер активной кабины
    uint8_t cab_idx = 0;

    const double RB_PRESS_DELAY = 1.5;

    /// Тамер выдержки РБ
    Timer *rb_timer = new Timer(RB_PRESS_DELAY, false);

    /// Заданная скорость
    double v_ref = 0.0;

    /// Конструкционная скорость
    double v_constr = 0.0;

    auto_feedback_t *feedback = nullptr;

    /// Переопределяем эту реализацию пустой, так как её может и не быть
    /// (что вряд ли, конечно...)
    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override
    {

    }

    /// Контроль бдительности
    virtual void vigilance_control(double t, double dt);

    /// Обработка РБ
    virtual void onPressRB_Timeout()
    {

    }

    virtual void press_RB()
    {

    }

    void load_config(CfgReader &cfg) override;

private slots:

    void slotVigilanceControl();
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef Autopilot* (*GetAutopilot)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AUTOPILOT(ClassName) \
    extern "C" Q_DECL_EXPORT Autopilot *getAutopilot() \
    {\
        return new (ClassName) ();\
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DEVICE_EXPORT Autopilot *loadAutopilot(QString lib_path);

#endif
