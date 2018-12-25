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
    OUTSIZE_VIEW
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

    /// Camera view setting
    void setCameraView(CameraView cameraView, osg::Camera *camera);
};

#endif // SWITCH_VIEW_H
