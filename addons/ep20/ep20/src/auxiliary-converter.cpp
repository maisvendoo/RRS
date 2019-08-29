#include    "auxiliary-converter.h"

AuxiliaryConverter::AuxiliaryConverter(QObject *parent) : Device (parent)
  ,U4(0)
  ,Fref(1)
  ,Uref(1)
  ,Koef(0.1267)
  ,U1(0)
  ,F(0)
  ,U2(0)
{

}

AuxiliaryConverter::~AuxiliaryConverter()
{

}

void AuxiliaryConverter::setU4(double U4)
{
    this->U4 = U4;
}

double AuxiliaryConverter::getU1()
{
    return  U1;
}

double AuxiliaryConverter::getF()
{
    return  F;
}

double AuxiliaryConverter::getU2()
{
    return U2;
}

void AuxiliaryConverter::preStep(state_vector_t &Y, double t)
{
    U1 = Uref;

    F = Fref;

    U2 = Koef * U4;
}

void AuxiliaryConverter::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{

}

void AuxiliaryConverter::load_config(CfgReader &cfg)
{

}
