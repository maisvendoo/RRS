#ifndef     TSKBM_H
#define     TSKBM_H

#include    "abstract-device-TSKBM.h"
#include    "timer.h"

class TSKBM final : public AbstractDeviceTSKBM
{

public:

    explicit TSKBM(QObject *parent = nullptr);
    ~TSKBM() = default;

    void step(double t, double dt) override;

    /// Запуск таймера периодической проверки бдительности ТСКБМ
    void startSafetyTimer() const override;

    /// Остановка таймера периодической проверки бдительности ТСКБМ
    void stopSafetyTimer() const override;

    void load_config(CfgReader &cfg) override;

private:

    struct TSKBM_cfg
    {
        /// Минимальный интервал периодической проверки бдительности ТСКБМ при работающем устройстве
        int    min_check_interval_on_device = 0;
        /// Максимальный интервал периодической проверки бдительности ТСКБМ при работающем устройстве
        int    max_check_interval_on_device = 0;
        /// Интервал до срыва ЭПК при проверке бдительности ТСКБМ
        double failure_EPK_interval = 0.0;

        TSKBM_cfg() = default;
    };

    TSKBM_cfg tskbm_cfg;

    /// Таймер периодической проверки бдительности ТСКБМ
    Timer *safety_timer = new Timer(220.0, false, this);

    /// Таймер срыва ЭПК при периодической проверке бдительности ТСКБМ
    Timer *failure_EPK_timer = new Timer(5.0, false, this);

    /// Запуск таймера срыва ЭПК при периодической проверке бдительности ТСКБМ
    void startFailureEPKTimer() const;
    /// Остановка таймера срыва ЭПК при периодической проверке бдительности ТСКБМ
    void stopFailureEPKTimer() const;

    /// Вернуть интервал таймера периодической проверки бдительности ТСКБМ
    int getSafetyTimerInterval(int min, int max) const;

private slots:
    /// Обработчик таймера периодической проверки бдительности ТСКБМ
    void onSafetyTimer();

    /// Обработчик таймера срыва ЭПК при периодической проверке бдительности ТСКБМ
    void onFailureEPKTimer();
};

#endif // TSKBM_H
