#include    "connector-ALSN.h"
#include    "trajectory-ALSN.h"
#include    "switch.h"
#include    "train-signal.h"
#include    <core/get_module.h>

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
void ConnectorALSN::step(double t, double dt)
{
    (void) t;
    (void) dt;

    // Код АЛСН из светофора вперёд для частотного сигнала у траектории сзади
    if (TrainSignal* signal_fwd = dynamic_cast<TrainSignal*>(connector->getSignalFwd()))
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
            traj_device->setNextSignalInfo(-bwd_dir, code, 0.0, signal_fwd->getLetter());
        }
    }

    // Код АЛСН из светофора назад для частотного сигнала у траектории спереди
    if (TrainSignal* signal_bwd = dynamic_cast<TrainSignal*>(connector->getSignalBwd()))
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
            traj_device->setNextSignalInfo(fwd_dir, code, 0.0, signal_bwd->getLetter());
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

GET_MODULE(ConnectorALSN)
