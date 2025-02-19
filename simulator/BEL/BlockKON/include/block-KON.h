#ifndef     BLOCKKON_H
#define     BLOCKKON_H

#include    "abstract-block-KON.h"
#include    "timer.h"

class BlockKON final : public AbstractBlockKON
{

public:

    explicit BlockKON(QObject *parent = nullptr);
    ~BlockKON() = default;

    void step(double t, double dt) override;

    void setKeyEPK(bool key_epk) override;

    void setVelocityKmh(double v_kmh) override;

    void setBrakeCylinderPressure(double pBC) override;

    bool getValveElectricalSupply() const override;

    void load_config(CfgReader &cfg) override;

private:

    Timer   *control_timer = new Timer(11.0, false, this);
    double  v_kmh = 0.0;
    double  pBC = 0.0;
    bool    key_epk = false;
    bool    old_key_epk = false;
    bool    valve_electrical_supply = false;

    struct block_KON_cfg
    {
        /// Допустимое давление в тормозных цилиндрах в случае выключения ЭПК
        double  permissible_pressure_BC = 0.0;
        /// Интервал анализа величины давления после выключения ЭПК
        int     control_interval = 0;

        block_KON_cfg() = default;
    };

    block_KON_cfg block_kon_cfg;

    /// Проверка на движение поезда
    bool isMove();

    /// Проверка на отключение ЭПК
    bool isEPKShutdown();

    /// Проверка на включение ЭПК
    bool isOnEPK();

    /// Запуск таймера блока контроля несанкционированного отключения ЭПК
    void startControlTimer() const;

    /// Остановка таймера блока контроля несанкционированного отключения ЭПК
    void stopControlTimer() const;

private slots:

    void onControlTimer();
};

#endif // BLOCKKON_H
