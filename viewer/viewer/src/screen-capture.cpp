#include    "screen-capture.h"

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ScreenCaptureHandler::ScreenCaptureHandler(osgViewer::ScreenCaptureHandler *sch)
    : sch(sch)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ScreenCaptureHandler::handle(const osgGA::GUIEventAdapter &ea,
                                  osgGA::GUIActionAdapter &aa)
{
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYUP:
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_F12)
            {
                osgViewer::Viewer *viewer = static_cast<osgViewer::Viewer *>(&aa);
                sch->setFramesToCapture(1);
                sch->captureNextFrame(*viewer);
                viewer->frame();
                sch->stopCapture();
            }

            break;
        }

    default:

        break;
    }

    return false;
}
