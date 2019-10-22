#include    "slave.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Slave::Slave(QObject *parent) : QObject(parent)
  , id(0)
  , is_config_required(false)
  , cells_mask(0)
  , description("")
  , is_connected(true)
  , errors(0)
  , maxErrors(10)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Slave::~Slave()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Slave::load_config(QString cfg_path)
{
    if (cfg_path.isEmpty())
        return false;

    CfgReader   cfg;

    if (cfg.load(cfg_path))
    {
        load_data_structure("DiscreteInput", cfg, discrete_input);
        load_data_structure("Coil", cfg, coil);
        load_data_structure("InputRegister", cfg, input_register);
        load_data_structure("HoldingRegister", cfg, holding_register);
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Slave::load_data_structure(QString name,
                                CfgReader &cfg,
                                data_map_t &data)
{
    QDomNode secNode = cfg.getFirstSection(name);

    while (!secNode.isNull())
    {
        slave_data_t data_unit;

        int tmp = 0;

        if (cfg.getInt(secNode, "Address", tmp))
        {
            data_unit.address = static_cast<quint16>(tmp);
        }

        if (cfg.getInt(secNode, "Index", tmp))
        {
            data_unit.index = static_cast<size_t>(tmp);
        }

        data.insert(data_unit.address, data_unit);

        secNode = cfg.getNextSection();
    }
}
