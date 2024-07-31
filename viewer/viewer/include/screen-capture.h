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
class ScreenCapture : public osgViewer::ScreenCaptureHandler
{
public:

    ScreenCapture(osgViewer::ScreenCaptureHandler::CaptureOperation* write_op);

    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

private:

    bool is_Shift_L = false;
    bool is_Shift_R = false;
    bool is_Ctrl_L = false;
    bool is_Ctrl_R = false;
    bool is_Alt_L = false;
    bool is_Alt_R = false;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class WriteToFileOperation : public osgViewer::ScreenCaptureHandler::CaptureOperation
{
public:

    WriteToFileOperation(const std::string &screenshotsDir);


    virtual void operator()(const osg::Image &image, const unsigned int context_id);


private:

    std::string screenshotsDir;
};

#endif // SCREEN_CAPTURE_H
