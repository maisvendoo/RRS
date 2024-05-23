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
    PneumoHose(QObject *parent = Q_NULLPTR);

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

    /// Задать длину рукава, м
    void setLength(double value);

    /// Задать смещение точки крепления рукава в сторону, м
    void setShiftSide(double value);

    /// Задать координату точки крепления руква на треке пути, м
    void setCoord(double value);

    /// Получить поток через рукав
    double getFlow() const;

    /// Получить угол отклонения рукава в сторону соседнего, радиан
    double getSideAngle() const;

    /// Получить угол свешивания рукава вниз с учётом натяжения от соседнего, радиан
    double getDownAngle() const;

    virtual void step(double t, double dt);

private:

    /// Флаг вызова команд управления соединением рукавов
    bool is_ref_state_command;

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg);

};

/*!
 * \typedef
 * \brief getPneumoHose() signature
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef PneumoHose* (*GetPneumoHose)();

/*!
 * \def
 * \brief Macro for getPneumoHose() generation
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_PNEUMO_HOSE(ClassName) \
    extern "C" Q_DECL_EXPORT PneumoHose *getPneumoHose() \
    {\
        return new (ClassName)(); \
    }

/*!
 * \fn
 * \brief Load PneumoHose from library
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT PneumoHose *loadPneumoHose(QString lib_path);

#endif // PNEUMO_HOSE_H
