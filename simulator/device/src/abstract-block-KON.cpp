#include    "abstract-block-KON.h"
#include    <QLibrary>

AbstractBlockKON::AbstractBlockKON(QObject *parent) : Device(parent)
{

}

AbstractBlockKON::~AbstractBlockKON()
{

}

void AbstractBlockKON::step(double t, double dt)
{
    Device::step(t, dt);
}

void AbstractBlockKON::ode_system(const state_vector_t &Y, state_vector_t &dYdt, double t)
{
    (void) Y;
    (void) dYdt;
    (void) t;
}

AbstractBlockKON *loadPluginBlockKON(QString lib_path)
{
    AbstractBlockKON *plugin_block_KON = nullptr;

    QLibrary lib(lib_path);

    if(lib.load())
    {
        GetPluginBlockKON getPluginBlockKON = reinterpret_cast<GetPluginBlockKON>(lib.resolve("getPluginBlockKON"));

        if(getPluginBlockKON)
        {
            plugin_block_KON = getPluginBlockKON();
        }
    }

    return plugin_block_KON;
}
