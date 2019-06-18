#include    "openvr-input-handler.h"

#include    <osgViewer/Viewer>
#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
OpenVRInputHandler::OpenVRInputHandler(OpenVRDevice *device) : osgGA::GUIEventHandler()
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
bool OpenVRInputHandler::handle(const osgGA::GUIEventAdapter &ea,
                                 osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {


            break;
        }
    }


    return false;
}
