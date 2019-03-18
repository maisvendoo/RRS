//------------------------------------------------------------------------------
//
//      Control of camera viewes
//      (c) maisvendoo, 23/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Control of camera viewes
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 23/12/2018
 */

#ifndef     SWITCH_VIEW_H
#define     SWITCH_VIEW_H

#include    <osgGA/GUIEventHandler>

/*!
 * \enum
 * \brief Camera views
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum CameraView
{
    CABINE_VIEW,
    OUTSIZE_VIEW,
    FREE_VIEW
};

/*!
 * \class
 * \brief Camera view handler
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class  CameraViewHandler : public osgGA::GUIEventHandler
{
public:

    /// Constructor
    CameraViewHandler();

    /// Handle method
    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

private:

    /// Current camera view
    CameraView cameraView;

    /// Vertical view angle
    double  angleVertical;

    /// Horizontal view angle
    double  angleHorizontal;

    /// External camera distance
    double  dist;

    /// Internal camera horizontal view angle
    double camAngleHorizontal;

    /// Internal camera vertical view angle
    double camAngleVertical;

    /// Initial view matrix for free camera
    osg::Matrix baseViewMatrix;

    osg::Vec3   free_shift;
    osg::Vec3   free_dir;

    double      free_angle_vert;
    double      free_andle_horiz;

    /// Camera view setting
    void setCameraView(CameraView cameraView, osg::Camera *camera);

    /// Outside camera motion
    void outCameraMotion(const osgGA::GUIEventAdapter &ea);

    /// Inside camera motion
    void intCameraMotion(const osgGA::GUIEventAdapter &ea);

    /// Free outside camera motion
    void freeCameraMotion(const osgGA::GUIEventAdapter &ea);
};

#endif // SWITCH_VIEW_H
