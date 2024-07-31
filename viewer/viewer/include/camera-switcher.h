#ifndef     CAMERA_SWITCHER_H
#define     CAMERA_SWITCHER_H

#include    <osgGA/KeySwitchMatrixManipulator>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class CameraSwitcher : public osgGA::KeySwitchMatrixManipulator
{
public:

    CameraSwitcher();

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:

    virtual ~CameraSwitcher();

    bool is_Shift_L = false;
    bool is_Shift_R = false;
    bool is_Ctrl_L = false;
    bool is_Ctrl_R = false;
    bool is_Alt_L = false;
    bool is_Alt_R = false;
};

#endif // CAMERASWITCHER_H
