#include    "openvr-events-handler.h"

#include    <osgViewer/Viewer>
#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
OpenVREventsHandler::OpenVREventsHandler(OpenVRDevice *device) : osgGA::GUIEventHandler()
  , device(device)
{
    vr::EVRInputError error = vr::VRInput()->SetActionManifestPath("/home/maisvendoo/actions.json");

    if (error != vr::EVRInputError::VRInputError_None)
    {
        std::cout << "Error" << std::endl;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool OpenVREventsHandler::handle(const osgGA::GUIEventAdapter &ea,
                                 osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {
            vr::VREvent_t event;

            while (device->getVrSystem()->PollNextEvent(&event, sizeof(event)))
            {
                processEvent(event);
            }

            break;
        }
    }


    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void OpenVREventsHandler::processEvent(vr::VREvent_t &event)
{


    switch (event.eventType)
    {
    case vr::VREvent_ButtonPress:
        {

            break;
        }
    }
}
