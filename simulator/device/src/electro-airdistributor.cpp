#include    "electro-airdistributor.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroAirDistributor::ElectroAirDistributor(QObject *parent) : BrakeDevice(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroAirDistributor::~ElectroAirDistributor()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ElectroAirDistributor *loadElectroAirDistributor(QString lib_path)
{

        ElectroAirDistributor *electro_airdist = nullptr;

        QLibrary lib(lib_path);

        if (lib.load())
        {
            GetElectroAirDistributor getElectroAirDistributor = reinterpret_cast<GetElectroAirDistributor>(lib.resolve("getElectroAirDistributor"));

            if (getElectroAirDistributor)
            {
                electro_airdist = getElectroAirDistributor();
            }
        }

        return electro_airdist;
}
