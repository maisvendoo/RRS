#include    "openvr-input-handler.h"

#include    <osgViewer/Viewer>
#include    <iostream>


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
OpenVRInputHandler::OpenVRInputHandler(OpenVRDevice *device, const std::string &action_manifest_path) : osgGA::GUIEventHandler()
  , device(device)
{
    const char *path = action_manifest_path.c_str();
    vr::EVRInputError error = vr::VRInput()->SetActionManifestPath(path);

    if (error != vr::EVRInputError::VRInputError_None)
    {
        osg::notify(osg::WARN) << "Error: Failed set action manifest" << std::endl;
    }

    vr::VRInput()->GetActionHandle("/actions/suggested/in/IncTraction", &m_actionIncTraction);
    vr::VRInput()->GetActionHandle("/actions/suggested/in/DecTraction", &m_actionDecTraction);

    vr::VRInput()->GetActionSetHandle("/actions/suggested", &m_actionset);

    vr::VRInput()->GetInputSourceHandle("/user/hand/left", &m_leftHand_source);
    vr::VRInput()->GetInputSourceHandle("/user/hand/right", &m_rightHand_source);

    vr::VRInput()->GetActionHandle("/actions/suggested/in/Hand_Left", &m_action_Hand_Left);
    vr::VRInput()->GetActionHandle("/actions/suggested/in/Hand_Right", &m_action_Hand_Right);

    vr::VRInput()->GetActionHandle("/actions/suggested/in/IncBrake", &m_actionIncBrake);
    vr::VRInput()->GetActionHandle("/actions/suggested/in/DecBrake", &m_actionDecBrake);

    vr::VRInput()->GetActionHandle("/actions/suggested/in/Reset", &m_Reset);

    vr::VRInput()->GetActionHandle("/actions/suggested/in/ReturnGV", &m_ReturnGV);

    vr::VRInput()->GetActionHandle("/actions/suggested/in/Start", &m_Start);
}

bool GetDigitalActionState(vr::VRActionHandle_t action, bool &state)
{
    vr::InputDigitalActionData_t actionData;
    vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof (actionData), vr::k_ulInvalidInputValueHandle);

    state = actionData.bState;

    return actionData.bActive;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool OpenVRInputHandler::handle(const osgGA::GUIEventAdapter &ea,
                                 osgGA::GUIActionAdapter &aa)
{
    osgViewer::Viewer *viewer = static_cast<osgViewer::Viewer *>(&aa);

    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {
            vr::VRActiveActionSet_t actionSet = { 0 };
            actionSet.ulActionSet = m_actionset;
            vr::VRInput()->UpdateActionState( &actionSet, sizeof(actionSet), 1 );

            bool state = false;

            if (GetDigitalActionState(m_actionIncTraction, state))
            {
                if (state)
                    viewer->getEventQueue()->keyPress(0, osgGA::GUIEventAdapter::KEY_A);
                else
                    viewer->getEventQueue()->keyRelease(0, osgGA::GUIEventAdapter::KEY_A);
            }


            if (GetDigitalActionState(m_actionDecTraction, state))
            {
                if (state)
                    viewer->getEventQueue()->keyPress(0, osgGA::GUIEventAdapter::KEY_D);
                else
                    viewer->getEventQueue()->keyRelease(0, osgGA::GUIEventAdapter::KEY_D);
            }

            if (GetDigitalActionState(m_actionIncBrake, state))
            {
                if (state)
                    viewer->getEventQueue()->keyPress(0, osgGA::GUIEventAdapter::KEY_Quote);
                else
                    viewer->getEventQueue()->keyRelease(0, osgGA::GUIEventAdapter::KEY_Quote);
            }

            if (GetDigitalActionState(m_actionDecBrake, state))
            {
                if (state)
                    viewer->getEventQueue()->keyPress(0, osgGA::GUIEventAdapter::KEY_Semicolon);
                else
                    viewer->getEventQueue()->keyRelease(0, osgGA::GUIEventAdapter::KEY_Semicolon);
            }

            if (GetDigitalActionState(m_Reset, state))
            {
                if (state)
                    device->resetSensorOrientation();
            }

            if (GetDigitalActionState(m_ReturnGV, state))
            {
                if (state)
                    viewer->getEventQueue()->keyPress(0, osgGA::GUIEventAdapter::KEY_B);
                else
                    viewer->getEventQueue()->keyRelease(0, osgGA::GUIEventAdapter::KEY_B);
            }

            if (GetDigitalActionState(m_Start, state))
            {
                if (state)
                    viewer->getEventQueue()->keyPress(0, osgGA::GUIEventAdapter::KEY_Space);
                else
                    viewer->getEventQueue()->keyRelease(0, osgGA::GUIEventAdapter::KEY_Space);
            }

            vr::InputPoseActionData_t rPoseData;
            vr::VRInput()->GetPoseActionDataForNextFrame(m_action_Hand_Right, vr::TrackingUniverseStanding, &rPoseData, sizeof (rPoseData), vr::k_ulInvalidInputValueHandle);

            vr::InputPoseActionData_t lPoseData;
            vr::VRInput()->GetPoseActionDataForNextFrame(m_action_Hand_Left, vr::TrackingUniverseStanding, &lPoseData, sizeof (lPoseData), vr::k_ulInvalidInputValueHandle);

            break;
        }
    }

    return false;
}
