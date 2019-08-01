#include <fstream>
#include <iostream>

#include "mpcs.h"
#include "current-kind.h"


MPCS::MPCS(QObject *parent) : Device(parent)
  , selectedCab(1)
  , last_current_kind(1)
{

//    readLastCurrentKind();
//    writeLastCurrentKind();
}

MPCS::~MPCS()
{

}

void MPCS::setSignalInputMPCS(const mpcs_input_t &mpcs_input)
{
    this->mpcs_input = mpcs_input;
}

void MPCS::setStoragePath(QString path)
{
    pathStorage = path;
}

void MPCS::readLastCurrentKind()
{
    ifstream file(pathStorage.toStdString() + "last_current_kind");

    if (file.is_open())
    {
        file >> last_current_kind;
        file.close();
    }
    else
    {
        last_current_kind = CURRENT_AC;
    }
}

void MPCS::writeLastCurrentKind()
{
    ofstream file(pathStorage.toStdString() + "last_current_kind");

    if (file.is_open())
    {
        file << last_current_kind;
        file.close();
    }
    else
    {
        last_current_kind = CURRENT_AC;
    }
}

mpcs_output_t MPCS::getSignalOutputMPCS()
{
    return mpcs_output;
}

void MPCS::init()
{
    pant_group_t dc_group;
    pant_group_t ac_group;

    switch (selectedCab)
    {
    case 1:

        dc_group[0] = PANT_DC2;
        dc_group[1] = PANT_DC1;
        ac_group[0] = PANT_AC2;
        ac_group[1] = PANT_AC1;
        break;

    case 2:
        dc_group[1] = PANT_DC2;
        dc_group[0] = PANT_DC1;
        ac_group[1] = PANT_AC2;
        ac_group[0] = PANT_AC1;

        break;
    }

    pants.insert(CURRENT_AC, ac_group);
    pants.insert(CURRENT_DC, dc_group);

    readLastCurrentKind();
}

void MPCS::preStep(state_vector_t &Y, double t)
{
    // Подъем приоритетного токоприеника данного рода тока
     mpcs_output.pant_state[pants[last_current_kind][0]] = true;


}

void MPCS::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{

}

void MPCS::load_config(CfgReader &cfg)
{

}
