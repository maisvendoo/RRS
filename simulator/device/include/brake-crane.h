//------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------
#ifndef     BRAKE_CRANE_H
#define     BRAKE_CRANE_H

#include    "brake-device.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    BP_PRESSURE = 0,
    ER_PRESSURE = 1
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT BrakeCrane : public BrakeDevice
{
public:

    BrakeCrane(QObject *parent = Q_NULLPTR);

    ~BrakeCrane();

    double getBrakePipeInitPressure() const;

    double getEqReservoirPressure() const;

    void setBrakePipeFlow(double Qbp);

    void setEqResrvoirFlow(double Qer);

    //
    void setChargePressure(double p0);

    //
    void setFeedLinePressure(double pFL);

    //
    void setBrakePipePressure(double pTM1);

    virtual void setPosition(int &position) = 0;

    virtual QString getPositionName() = 0;

    virtual float getHandlePosition() = 0;

protected:

    double Ver;

    double Vbp;

    double p0;

    double pFL;

    double pTM1;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);
private:

    double Qer;

    double Qbp;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef BrakeCrane* (*GetBrakeCrane)();

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_BRAKE_CRANE(ClassName) \
    extern "C" Q_DECL_EXPORT BrakeCrane *getBrakeCrane() \
    { \
        return new (ClassName) (); \
    }

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT BrakeCrane *loadBrakeCrane(QString lib_path);

#endif // BRAKE_CRANE_H
