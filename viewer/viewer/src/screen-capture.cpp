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
    }
    default: break;
    }

    if (is_Shift_L || is_Shift_R || is_Ctrl_L || is_Ctrl_R || is_Alt_L || is_Alt_R)
        return false;

    return osgViewer::ScreenCaptureHandler::handle(ea, aa);
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
