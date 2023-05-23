#ifndef     JOINT_H
#define     JOINT_H

#include    <QString>

#include    "device-export.h"
#include    "device-list.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class DEVICE_EXPORT Joint
{
public:

    /// Constructor
    Joint();
    /// Destructor
    virtual ~Joint();

    /// Read joint config file
    virtual void read_config(const QString &path);

    /// Set connector for linkage
    void setLink(Device *device, size_t idx = 0);

    /// Simulation step
    virtual void step(double t, double dt);

protected:

    /// List of linked devices
    device_list_t devices;

    /// Joint configuration loading
    virtual void load_config(CfgReader &cfg);
};

/*!
 * \typedef
 * \brief getJoint() signature
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef Joint* (*GetJoint)();

/*!
 * \def
 * \brief Macro for getJoint() generation
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_JOINT(ClassName) \
    extern "C" Q_DECL_EXPORT Joint *getJoint() \
    {\
        return new (ClassName)(); \
    }

/*!
 * \fn
 * \brief Load joint from library
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT Joint *loadJoint(QString lib_path);

#endif // COUPLING_H
