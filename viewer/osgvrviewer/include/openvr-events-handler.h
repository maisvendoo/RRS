#ifndef     OPENVR_EVENTS_HANDLER_H
#define     OPENVR_EVENTS_HANDLER_H

#include    <osgGA/GUIEventHandler>
#include    <openvr/openvr.h>

#include    "openvrdevice.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVREventsHandler : public osgGA::GUIEventHandler
{
public:

    OpenVREventsHandler(OpenVRDevice *device);

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

protected:

    OpenVRDevice    *device;

    void processEvent(vr::VREvent_t &event);
};

#endif // OPENVREVENTSHANDLER_H
