#ifndef     SCREEN_CAPTURE_H
#define     SCREEN_CAPTURE_H

#include    <osgViewer/Viewer>
#include    <osgViewer/ViewerEventHandlers>
#include    <osgDB/WriteFile>
#include    <osgGA/GUIEventHandler>

#include    <QDir>
#include    <QDateTime>
#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class WriteToFileOperation : public osgViewer::ScreenCaptureHandler::CaptureOperation
{
public:

    WriteToFileOperation(const std::string &screenshotsDir)
    {
        this->screenshotsDir = screenshotsDir;
    }

    virtual void operator()(const osg::Image &image, const unsigned int context_id)
    {
        std::string fileName = screenshotsDir +
                QDir::separator().toLatin1() +
                "screenshot-" +
                QDateTime::currentDateTime().toString().toStdString() + ".png";

        osgDB::writeImageFile(image, fileName);
    }

private:

    std::string screenshotsDir;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ScreenCaptureHandler : public osgGA::GUIEventHandler
{
public:

    ScreenCaptureHandler(osgViewer::ScreenCaptureHandler *sch)
    {
        this->sch = sch;
    }

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
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

private:

    osgViewer::ScreenCaptureHandler *sch;
};

#endif // SCREEN_CAPTURE_H
