#ifndef     OPENVR_INPUT_HANDLER_H
#define     OPENVR_INPUT_HANDLER_H

#include    <osgGA/GUIEventHandler>
#include    <openvr/openvr_mingw.hpp>

#include    "openvrdevice.h"



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class OpenVRInputHandler : public osgGA::GUIEventHandler
{
public:

    OpenVRInputHandler(OpenVRDevice *device, const std::string &action_manifest_path);

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

protected:

    OpenVRDevice    *device;

    vr::VRActionHandle_t m_actionIncTraction;
    vr::VRActionHandle_t m_actionDecTraction;
    vr::VRActionHandle_t m_action_Hand_Left;
    vr::VRActionHandle_t m_action_Hand_Right;
    vr::VRActionHandle_t m_actionIncBrake;
    vr::VRActionHandle_t m_actionDecBrake;

    vr::VRActionSetHandle_t m_actionset;

    vr::VRInputValueHandle_t m_leftHand_source;
    vr::VRInputValueHandle_t m_rightHand_source;

    vr::HmdMatrix34_t   m_leftPose;
    vr::HmdMatrix34_t   m_rightPose;
};

#endif // OPENVR_INPUT_HANDLER_H
