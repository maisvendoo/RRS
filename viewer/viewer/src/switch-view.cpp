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

#include    "switch-view.h"

#include    <osgViewer/Viewer>
#include    <osg/Camera>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CameraViewHandler::CameraViewHandler() : osgGA::GUIEventHandler ()
  , cameraView(CABINE_VIEW)
  , angleVertical(0.0)
  , angleHorizontal(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool CameraViewHandler::handle(const osgGA::GUIEventAdapter &ea,
                               osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
        {

            osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);
            osg::Camera *camera = viewer->getCamera();

            setCameraView(cameraView, camera);
            break;
        }

    case osgGA::GUIEventAdapter::KEYDOWN:

        switch (ea.getKey())
        {
        case osgGA::GUIEventAdapter::KEY_1:

            cameraView = CABINE_VIEW;
            break;

        case osgGA::GUIEventAdapter::KEY_2:

            cameraView = OUTSIZE_VIEW;
            break;

        case osgGA::GUIEventAdapter::KEY_Left:
            angleVertical += 0.1;
            break;

        case osgGA::GUIEventAdapter::KEY_Right:
            angleVertical -= 0.1;
            break;

        case osgGA::GUIEventAdapter::KEY_Up:
            angleHorizontal += 0.1;
            break;

        case osgGA::GUIEventAdapter::KEY_Down:
            angleHorizontal -= 0.1;
            break;        
        }

        break;

    default:

        break;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraViewHandler::setCameraView(CameraView cameraView, osg::Camera *camera)
{
    osg::Matrix viewMatrix = camera->getViewMatrix();

    switch (cameraView)
    {
    case CABINE_VIEW:
        {

            break;
        }

    case OUTSIZE_VIEW:
        {
            osg::Matrix matrix;
            matrix *= osg::Matrix::rotate(osg::PI_2 + angleVertical, osg::Vec3(0, 1, 0));
            matrix *= osg::Matrix::rotate(angleHorizontal, osg::Vec3(1, 0, 0));            
            matrix *= osg::Matrix::translate(osg::Vec3(0.0, 0.0, -20.0));

            viewMatrix *= matrix;
            break;
        }
    }

    camera->setViewMatrix(viewMatrix);
}
