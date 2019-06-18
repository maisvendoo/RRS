#ifndef     OPENVR_INPUT_HANDLER_H
#define     OPENVR_INPUT_HANDLER_H

#include    <osgGA/GUIEventHandler>
#include    <openvr/openvr.h>

#include    "openvrdevice.h"



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRInputHandler : public osgGA::GUIEventHandler
{
public:

    OpenVRInputHandler(OpenVRDevice *device);

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

protected:

    OpenVRDevice    *device;


};

#endif // OPENVR_INPUT_HANDLER_H
