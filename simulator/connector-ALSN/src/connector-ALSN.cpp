#include    "connector-ALSN.h"
#include    "trajectory-ALSN.h"
#include    "connector.h"
#include    "rail-signal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorALSN::ConnectorALSN(QObject *parent) : ConnectorDevice(parent)
{
    name = QString("ALSN");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ConnectorALSN::~ConnectorALSN()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryDevice *ConnectorALSN::getFwdTrajectoryDevice() const
{
    // Если есть светофор назад, не пропускаем поиск по топологии
    // от предыдущих светофоров вперёд
    if (is_signal_bwd)
        return nullptr;

    return fwd_traj_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrajectoryDevice *ConnectorALSN::getBwdTrajectoryDevice() const
{
    // Если есть светофор вперёд, не пропускаем поиск по топологии
    // от следующих светофоров назад
    if (is_signal_fwd)
        return nullptr;

    return bwd_traj_device;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorALSN::step(double t, double dt)
{
    (void) t;
    (void) dt;

    Signal *signal_fwd = connector->getSignalFwd();
    if (signal_fwd == nullptr)
        is_signal_fwd = false;
    else
        is_signal_fwd = true;

    Signal *signal_bwd = connector->getSignalBwd();
    if (signal_bwd == nullptr)
        is_signal_bwd = false;
    else
        is_signal_bwd = true;

    // Код АЛСН из светофора вперёд для частотного сигнала у траектории сзади
    if (is_signal_fwd)
    {
        TrajectoryALSN *traj_device = dynamic_cast<TrajectoryALSN *>(bwd_traj_device);
        if (traj_device != nullptr)
        {
            ALSN code = ALSN::NO_CODE;
            alsn_state_t state = signal_fwd->getALSNstate();
            if (state[ALSN_RY_LINE])
            {
                code = ALSN::RED_YELLOW;
            }
            else
            {
                if (state[ALSN_Y_LINE])
                {
                    code = ALSN::YELLOW;
                }
                else
                {
                    if (state[ALSN_G_LINE])
                    {
                        code = ALSN::GREEN;
                    }
                }
            }
            traj_device->setSignalInfoFwd(code, 0.0, signal_fwd->getLetter());
        }
    }

    // Код АЛСН из светофора назад для частотного сигнала у траектории спереди
    if (is_signal_bwd)
    {
        TrajectoryALSN *traj_device = dynamic_cast<TrajectoryALSN *>(fwd_traj_device);
        if (traj_device != nullptr)
        {
            ALSN code = ALSN::NO_CODE;
            alsn_state_t state = signal_bwd->getALSNstate();
            if (state[ALSN_RY_LINE])
            {
                code = ALSN::RED_YELLOW;
            }
            else
            {
                if (state[ALSN_Y_LINE])
                {
                    code = ALSN::YELLOW;
                }
                else
                {
                    if (state[ALSN_G_LINE])
                    {
                        code = ALSN::GREEN;
                    }
                }
            }
            traj_device->setSignalInfoBwd(code, 0.0, signal_bwd->getLetter());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ConnectorALSN::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getString(secName, "Name", name);
}

GET_CONNECTOR_DEVICE(ConnectorALSN)
