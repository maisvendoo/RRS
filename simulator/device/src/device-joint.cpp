#include    <QLibrary>

#include    "device-joint.h"

#include    "filesystem.h"
#include    "CfgReader.h"
#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Joint::Joint()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Joint::~Joint()
{
    if (!devices.empty())
    {
        for (auto device : devices)
        {
            if (device != nullptr)
            {
                device->unlink();
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Joint::read_config(const QString &path)
{
    CfgReader cfg;

    if (cfg.load(path))
    {
        load_config(cfg);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Joint::setLink(Device *device, size_t idx)
{
    if (idx < devices.size())
    {
        if (devices[idx] != nullptr)
        {
            devices[idx]->unlink();
        }

        if (device != nullptr)
        {
            device->link();
            devices[idx] = device;
        }
        else
        {
            devices[idx] = nullptr;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Joint::step(double t, double dt)
{
    (void) t;
    (void) dt;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Joint::load_config(CfgReader &cfg)
{
    (void) cfg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Joint *loadJoint(QString lib_path)
{
    Joint *joint = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetJoint getJoint = reinterpret_cast<GetJoint>(lib.resolve("getJoint"));

        if (getJoint)
        {
            joint = getJoint();
        }
        else
        {
            Journal::instance()->error(lib.errorString());
        }
    }
    else
    {
        Journal::instance()->error(lib.errorString());
    }

    return joint;
}
