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

    }

    ~Autopilot()
    {

    }

    /// Выдать сигналы управления для органов управления ПС
    virtual auto_control_t *getControl() = 0;

    /// Получить обратную связь от систем ПС
    virtual void setFeedback(auto_feedback_t *feed_back) = 0;

    void on()
    {
        is_active = true;
    }

    void off()
    {
        is_active = false;
    }

protected:

    /// Признак активации
    bool is_active = false;

    /// Переопределяем эту реализацию пустой, так как её может и не быть
    /// (что вряд ли, конечно...)
    void ode_system(const state_vector_t &Y,
                    state_vector_t &dYdt,
                    double t) override
    {

    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef Autopilot* (*GetAutopilot)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_AUTOPILOT(ClassName) \
    extern "C" Q_DECL_EXPORT Autopilot *fetAutopilot() \
    {\
        return new (ClassName) ();\
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" DEVICE_EXPORT Autopilot *loadAutopilot(QString lib_path);

#endif
