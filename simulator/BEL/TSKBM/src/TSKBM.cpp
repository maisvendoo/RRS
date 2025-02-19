#include    "TSKBM.h"
#include    <QDebug>
#include    <QRandomGenerator>
#include    "CfgReader.h"

const QString TSKBM_CFG = "TSKBM-config";

TSKBM::TSKBM(QObject *parent) : AbstractDeviceTSKBM(parent)
{
    connect(safety_timer, &Timer::process, this, &TSKBM::onSafetyTimer);
    connect(failure_EPK_timer, &Timer::process, this, &TSKBM::onFailureEPKTimer);

    this->read_config(TSKBM_CFG);

    safety_timer->setTimeout(getSafetyTimerInterval(tskbm_cfg.min_check_interval_on_device,
                                                    tskbm_cfg.max_check_interval_on_device));
}

void TSKBM::step(double t, double dt)
{
    safety_timer->step(t, dt);
    failure_EPK_timer->step(t, dt);

    AbstractDeviceTSKBM::step(t, dt);
}

void TSKBM::startSafetyTimer() const
{
    if(!safety_timer->isStarted())
        safety_timer->start();
}

void TSKBM::stopSafetyTimer() const
{
    if(safety_timer->isStarted())
        safety_timer->stop();

    stopFailureEPKTimer();
}

void TSKBM::load_config(CfgReader &cfg)
{
    QString sec_name = "Device";

    if(!cfg.getInt(sec_name, "MinVigCheckIntervalOnDevice", tskbm_cfg.min_check_interval_on_device))
    {
        constexpr int DEFAULT_MIN_CHECK_INTERVAL_ON_DEVICE = 180;

        tskbm_cfg.min_check_interval_on_device = DEFAULT_MIN_CHECK_INTERVAL_ON_DEVICE;

        qWarning() << "Warning: Minimum interval for periodic vigilance check"
                   << "of TSKBM when the device is running configuration not found."
                   << "Default minimum interval for periodic vigilance check TSKBM:"
                   << DEFAULT_MIN_CHECK_INTERVAL_ON_DEVICE;
    }

    if(!cfg.getInt(sec_name, "MaxVigCheckIntervalOnDevice", tskbm_cfg.max_check_interval_on_device))
    {
        constexpr int DEFAULT_MAX_CHECK_INTERVAL_ON_DEVICE = 600;

        tskbm_cfg.max_check_interval_on_device = DEFAULT_MAX_CHECK_INTERVAL_ON_DEVICE;

        qWarning() << "Warning: Maximum interval for periodic vigilance check"
                   << "of TSCBM when the device is running configuration not found."
                   << "Default maximum interval for periodic vigilance check TSKBM:"
                   << DEFAULT_MAX_CHECK_INTERVAL_ON_DEVICE;
    }

    if(!cfg.getDouble(sec_name, "FailureEPKInterval", tskbm_cfg.failure_EPK_interval))
    {
        constexpr double DEFAULT_FAILURE_EPK_INTERVAL = 5.0;

        tskbm_cfg.failure_EPK_interval = DEFAULT_FAILURE_EPK_INTERVAL;

        qWarning() << "Warning: Failure EPK interval configuration not found."
                   << "Default failure EPK interval:" << DEFAULT_FAILURE_EPK_INTERVAL;
    }
}

void TSKBM::startFailureEPKTimer() const
{
    if(!failure_EPK_timer->isStarted())
        failure_EPK_timer->start();
}

void TSKBM::stopFailureEPKTimer() const
{
    if(failure_EPK_timer->isStarted())
        failure_EPK_timer->stop();
}

int TSKBM::getSafetyTimerInterval(int min, int max) const
{
    if(min < max)
        return QRandomGenerator::global()->bounded(min, max);
    else
        return max;
}

void TSKBM::onSafetyTimer()
{
    emit TSKBMVigilanceCheck();

    startFailureEPKTimer();

    safety_timer->setTimeout(getSafetyTimerInterval(tskbm_cfg.min_check_interval_on_device,
                                                    tskbm_cfg.max_check_interval_on_device));
}

void TSKBM::onFailureEPKTimer()
{
    emit TSKBMfailureEPK();

    stopFailureEPKTimer();
}

GET_PLUGIN_TSKBM_DEVICE(TSKBM)
