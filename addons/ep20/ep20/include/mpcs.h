#ifndef MPCS_H
#define MPCS_H

#include "device.h"
#include "mpcs-data.h"
#include "pant-description.h"

class MPCS : public Device
{
public:

    MPCS(QObject *parent = Q_NULLPTR);

    ~MPCS();

    void init();

    void setSignalInputMPCS(const mpcs_input_t &mpcs_input);

    void setStoragePath(QString path);

    mpcs_output_t getSignalOutputMPCS();

private:

    QString pathStorage;

    /// Выбор кабины машиниста
    int selectedCab;

    /// Последнее значение рода тока
    int last_current_kind;

    mpcs_input_t mpcs_input;

    mpcs_output_t mpcs_output;

    enum
    {
        NUM_PANTS_GROUP = 2
    };

    typedef std::array<size_t, NUM_PANTS_GROUP> pant_group_t;
    QMap<int, pant_group_t> pants;


    /// Предварительный шаг
    void preStep(state_vector_t &Y, double t);

    /// Вычисление напряжения
    void ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t);

    /// Загрузка конфига
    void load_config(CfgReader &cfg);

    void readLastCurrentKind();

    void writeLastCurrentKind();
};

#endif // MPCS_H
