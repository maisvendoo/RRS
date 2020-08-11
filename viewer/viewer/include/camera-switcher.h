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

protected:

    virtual ~CameraSwitcher();
};

#endif // CAMERASWITCHER_H
