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
  , dist(20.0)
  , camAngleHorizontal(0.0)
  , camAngleVertical(0.0)
  , baseViewMatrix(osg::Matrix())
  , free_shift(osg::Vec3())
  , free_dir(osg::Vec3())
  , free_angle_vert(0.0)
  , free_andle_horiz(0.0)
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

            // Switching camera view (internal/external)
            setCameraView(cameraView, camera);
            break;
        }

    case osgGA::GUIEventAdapter::KEYDOWN:

        switch (ea.getKey())
        {
        case osgGA::GUIEventAdapter::KEY_F2:

            cameraView = CABINE_VIEW;
            break;

        case osgGA::GUIEventAdapter::KEY_F3:

            cameraView = OUTSIZE_VIEW;
            break;

        case osgGA::GUIEventAdapter::KEY_F4:

            if (cameraView == OUTSIZE_VIEW)
            {
                cameraView = FREE_VIEW;

                osgViewer::Viewer *viewer = dynamic_cast<osgViewer::Viewer *>(&aa);
                baseViewMatrix = viewer->getCamera()->getViewMatrix();

                free_angle_vert = -angleVertical;
                free_andle_horiz = angleHorizontal;
            }

            break;

        default:

            switch (cameraView)
            {
            case OUTSIZE_VIEW:

                outCameraMotion(ea);
                break;

            case CABINE_VIEW:

                intCameraMotion(ea);
                break;

            case FREE_VIEW:

                freeCameraMotion(ea);
                break;
            }
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
            osg::Matrix matrix;
            matrix *= osg::Matrix::rotate(camAngleVertical, osg::Vec3(0, 1, 0));
            matrix *= osg::Matrix::rotate(camAngleHorizontal, osg::Vec3(1, 0, 0));

            viewMatrix *= matrix;
            break;
        }

    case OUTSIZE_VIEW:
        {
            osg::Matrix matrix;
            matrix *= osg::Matrix::rotate(osg::PI_2 + angleVertical, osg::Vec3(0, 1, 0));
            matrix *= osg::Matrix::rotate(angleHorizontal, osg::Vec3(1, 0, 0));            
            matrix *= osg::Matrix::translate(osg::Vec3(0.0, 0.0, -dist));

            viewMatrix *= matrix;
            break;
        }

    case FREE_VIEW:
        {
            osg::Matrix tmp;

            tmp *= osg::Matrix::translate(-free_shift);
            tmp *= osg::Matrix::rotate(free_angle_vert, osg::Y_AXIS);

            viewMatrix = baseViewMatrix * tmp;

            break;
        }
    }

    camera->setViewMatrix(viewMatrix);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraViewHandler::outCameraMotion(const osgGA::GUIEventAdapter &ea)
{
    switch (ea.getKey())
    {
        case osgGA::GUIEventAdapter::KEY_Left:
            angleVertical -= 0.1;
            break;

        case osgGA::GUIEventAdapter::KEY_Right:
            angleVertical += 0.1;
            break;

        case osgGA::GUIEventAdapter::KEY_Up:
            angleHorizontal += 0.1;
            break;

        case osgGA::GUIEventAdapter::KEY_Down:
            angleHorizontal -= 0.1;
            break;

        case osgGA::GUIEventAdapter::KEY_Equals:
            dist += 0.5;
            break;

        case osgGA::GUIEventAdapter::KEY_Minus:
            dist -= 0.5;
            if (dist <= 5.0) dist = 5.0;
            break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraViewHandler::intCameraMotion(const osgGA::GUIEventAdapter &ea)
{
    switch (ea.getKey())
    {
        case osgGA::GUIEventAdapter::KEY_Left:
            camAngleVertical -= 0.02;
            break;

        case osgGA::GUIEventAdapter::KEY_Right:
            camAngleVertical += 0.02;
            break;

        case osgGA::GUIEventAdapter::KEY_Up:
            camAngleHorizontal -= 0.02;
            break;

        case osgGA::GUIEventAdapter::KEY_Down:
            camAngleHorizontal += 0.02;
            break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraViewHandler::freeCameraMotion(const osgGA::GUIEventAdapter &ea)
{
    double alpha = osg::PI_2 + free_angle_vert;
    free_dir = osg::Vec3d(cos(alpha), 0.0, sin(alpha));
    osg::Vec3 up(0.0, 1.0, 0.0);
    osg::Vec3 traverse = up ^ free_dir;

    switch (ea.getKey())
    {
    case osgGA::GUIEventAdapter::KEY_Left:

        if (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL)
        {
            free_angle_vert -= 0.01;
        }
        else
        {
            free_shift += traverse * 0.1f;
        }

        break;

    case osgGA::GUIEventAdapter::KEY_Right:

        if (ea.getModKeyMask() == osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL)
        {
            free_angle_vert += 0.01;
        }
        else
        {
            free_shift -= traverse * 0.1f;
        }

        break;

    case osgGA::GUIEventAdapter::KEY_Insert:

        free_shift += up * 0.1f;
        break;

    case osgGA::GUIEventAdapter::KEY_Delete:

        free_shift -= up * 0.1f;
        break;

    case osgGA::GUIEventAdapter::KEY_Up:

        free_shift -= free_dir * 0.3f;
        break;

    case osgGA::GUIEventAdapter::KEY_Down:

        free_shift += free_dir * 0.3f;
        break;
    }
}
