#ifndef     PNEUMO_HOSE_EPB_H
#define     PNEUMO_HOSE_EPB_H

#include    "pneumo-hose.h"

//------------------------------------------------------------------------------
// Рукав пневматической магистрали с линиями управления ЭПТ
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoHoseEPB : public PneumoHose
{
public:

    /// Конструктор
    PneumoHoseEPB(int key_code = 0, QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~PneumoHoseEPB();

    /// Задать число линий управления ЭПТ
    void setLinesNumber(size_t num);

    /// Число линий управления ЭПТ
    size_t getLinesNumber() const;

    /// Число соединённых линий управления ЭПТ
    size_t getConnectedLinesNumber() const;

    /// Задать величину напряжения в линии idx
    void setVoltage(size_t idx, double value);

    /// Задать частоту переменного напряжения в линии idx
    void setFrequency(size_t idx, double value);

    /// Задать потребляемый ток из линии idx
    void setCurrent(size_t idx, double value);

    /// Величина напряжения в линии idx
    double getVoltage(size_t idx);

    /// Частота переменного напряжения в линии idx
    double getFrequency(size_t idx);

    /// Потребляемый ток из линии idx
    double getCurrent(size_t idx);

    virtual void step(double t, double dt);

protected:

    /// Число линий управления ЭПТ
    size_t lines_num;

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg);

};

/*!
 * \typedef
 * \brief getPneumoHoseEPB() signature
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef PneumoHoseEPB* (*GetPneumoHoseEPB)();

/*!
 * \def
 * \brief Macro for getPneumoHoseEPB() generation
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_PNEUMO_HOSE_EPB(ClassName) \
    extern "C" Q_DECL_EXPORT PneumoHoseEPB *getPneumoHoseEPB() \
    {\
        return new (ClassName)(); \
    }

/*!
 * \fn
 * \brief Load PneumoHoseEPB from library
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT PneumoHoseEPB *loadPneumoHoseEPB(QString lib_path);

#endif // PNEUMO_HOSE_EPB_H
