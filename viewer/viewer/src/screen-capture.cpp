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
