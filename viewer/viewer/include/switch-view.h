//------------------------------------------------------------------------------
//
//
//
//
//------------------------------------------------------------------------------

#ifndef     SWITCH_VIEW_H
#define     SWITCH_VIEW_H

#include    <osgGA/GUIEventHandler>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum CameraView
{
    CABINE_VIEW,
    OUTSIZE_VIEW
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class  CameraViewHandler : public osgGA::GUIEventHandler
{
public:

    CameraViewHandler();

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

private:

    CameraView cameraView;

    double  angleVertical;

    double  angleHorizontal;

    double  long_shift;

    void setCameraView(CameraView cameraView, osg::Camera *camera);
};

#endif // SWITCH_VIEW_H
