#ifndef     PNEUMO_HOSE_H
#define     PNEUMO_HOSE_H

#include    "device.h"

#include    "pneumo-hose-data.h"

//------------------------------------------------------------------------------
// Рукав пневматической магистрали
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoHose : public Device
{
public:

    /// Конструктор
    PneumoHose();

    /// Деструктор
    ~PneumoHose();

    /// Соединить рукава
    void connect();

    /// Разъединить рукава
    void disconnect();

    /// Состояние соединения рукавов
    bool isConnected() const;

    /// Задать давление в рукаве
    void setPressure(double value);

    /// Задать коэффициент перетока через рукав
    void setFlowCoeff(double value);

    /// Получить поток через рукав
    double getFlow() const;

private:

    /// Флаг вызова команд управления соединением рукавов
    bool is_ref_state_command;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void preStep(state_vector_t &Y, double t);

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg);

};

#endif // PNEUMO_HOSE_H
