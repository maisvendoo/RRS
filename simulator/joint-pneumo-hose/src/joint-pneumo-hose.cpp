#include    "joint-pneumo-hose.h"

#include    "CfgReader.h"
#include    "physics.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointPneumoHose::JointPneumoHose() : Joint()
  , is_connected(false)
{
    devices.resize(NUM_CONNECTORS);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
JointPneumoHose::~JointPneumoHose()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHose::step(double t, double dt)
{
    (void) t;
    (void) dt;

    // Управление соединением рукавов
    if (   (devices[FWD]->getOutputSignal(HOSE_REF_STATE) == 1.0)
        || (devices[BWD]->getOutputSignal(HOSE_REF_STATE) == 1.0) )
    {
        is_connected = true;
    }

    // Управление разъединением рукавов
    if (   (devices[FWD]->getOutputSignal(HOSE_REF_STATE) == -1.0)
        || (devices[BWD]->getOutputSignal(HOSE_REF_STATE) == -1.0) )
    {
        is_connected = false;
    }

    // Расчёт потоков через рукава
    double p_fwd = devices[FWD]->getOutputSignal(HOSE_PIPE_PRESSURE);
    double p_bwd = devices[BWD]->getOutputSignal(HOSE_PIPE_PRESSURE);

    double flow_coeff_fwd = devices[FWD]->getOutputSignal(HOSE_FLOW_COEFF);
    double flow_coeff_bwd = devices[BWD]->getOutputSignal(HOSE_FLOW_COEFF);

    if (is_connected)
    {
        devices[FWD]->setInputSignal(HOSE_IS_CONNECTED, 1.0);
        devices[BWD]->setInputSignal(HOSE_IS_CONNECTED, 1.0);

        // Поток через соединённые рукава
        double flow_coeff = min(flow_coeff_fwd, flow_coeff_bwd);
        double flow = flow_coeff * (p_fwd - p_bwd);

        devices[FWD]->setInputSignal(HOSE_FLOW_TO_PIPE, -flow);
        devices[BWD]->setInputSignal(HOSE_FLOW_TO_PIPE, flow);
    }
    else
    {
        devices[FWD]->setInputSignal(HOSE_IS_CONNECTED, 0.0);
        devices[BWD]->setInputSignal(HOSE_IS_CONNECTED, 0.0);

        // Поток из разъединённых рукавов в атмосферу
        double flow = flow_coeff_fwd * p_fwd;
        devices[FWD]->setInputSignal(HOSE_FLOW_TO_PIPE, -flow);

        flow = flow_coeff_bwd * p_bwd;
        devices[BWD]->setInputSignal(HOSE_FLOW_TO_PIPE, -flow);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void JointPneumoHose::load_config(CfgReader &cfg)
{
    QString secName = "Joint";
    cfg.getBool(secName, "initConnectionState", is_connected);
}

GET_JOINT(JointPneumoHose)
