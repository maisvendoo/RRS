#include    "screen-capture.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ScreenCapture::ScreenCapture(osgViewer::ScreenCaptureHandler::CaptureOperation* write_op)
    : osgViewer::ScreenCaptureHandler(write_op)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ScreenCapture::handle(const osgGA::GUIEventAdapter &ea,
                           osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
/* Вроде не нужно, после отключения дефолтного поведения
 * непрерывная запись больше не включалась
    case osgGA::GUIEventAdapter::FRAME:
    {
        // Отключаем непрерывное создание скриншотов, если оно вдруг включилось
        if (getFramesToCapture() < 0)
        {
            setFramesToCapture(0);

            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            if (!view) return false;
            osgViewer::ViewerBase* viewer = view->getViewerBase();
            if (!viewer) return false;

            removeCallbackFromViewer(*viewer);
        }
    }
*/
    case osgGA::GUIEventAdapter::KEYDOWN:
    {
        int key = ea.getUnmodifiedKey();
        switch (key)
        {
        case osgGA::GUIEventAdapter::KEY_Shift_L:
            is_Shift_L = true;
            return false;
        case osgGA::GUIEventAdapter::KEY_Shift_R:
            is_Shift_R = true;
            return false;
        case osgGA::GUIEventAdapter::KEY_Control_L:
            is_Ctrl_L = true;
            return false;
        case osgGA::GUIEventAdapter::KEY_Control_R:
            is_Ctrl_R = true;
            return false;
        case osgGA::GUIEventAdapter::KEY_Alt_L:
            is_Alt_L = true;
            return false;
        case osgGA::GUIEventAdapter::KEY_Alt_R:
            is_Alt_R = true;
            return false;
        default: break;
        }
    }

    case osgGA::GUIEventAdapter::KEYUP:
    {
        int key = ea.getUnmodifiedKey();
        switch (key)
        {
        case osgGA::GUIEventAdapter::KEY_Shift_L:
            is_Shift_L = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Shift_R:
            is_Shift_R = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Control_L:
            is_Ctrl_L = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Control_R:
            is_Ctrl_R = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Alt_L:
            is_Alt_L = false;
            break;
        case osgGA::GUIEventAdapter::KEY_Alt_R:
            is_Alt_R = false;
            break;
        default: break;
        }

        if (is_Shift_L || is_Shift_R || is_Ctrl_L || is_Ctrl_R || is_Alt_L || is_Alt_R)
            return false;

        if (ea.getKey() == _keyEventTakeScreenShot)
        {
            osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
            if (!view) return false;
            osgViewer::ViewerBase* viewer = view->getViewerBase();
            if (!viewer) return false;

            setFramesToCapture(1);
            addCallbackToViewer(*viewer);
            return true;
        }
    }
    default: break;
    }

    //return osgViewer::ScreenCaptureHandler::handle(ea, aa);
    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
WriteToFileOperation::WriteToFileOperation(const std::string &screenshotsDir)
    : screenshotsDir(screenshotsDir)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void WriteToFileOperation::operator()(const osg::Image &image,
                                      const unsigned int context_id)
{
    (void) context_id;

    std::string fileName = screenshotsDir +
            QDir::separator().toLatin1() +
            "screenshot_" +
            QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss").toStdString() + ".jpeg";

    osgDB::writeImageFile(image, fileName);
}
