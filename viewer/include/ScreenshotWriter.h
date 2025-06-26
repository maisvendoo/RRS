#pragma once
#ifndef SCREENSHOT_WRITER_H
#define SCREENSHOT_WRITER_H

#include "vsg/app/Window.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ScreenshotWriter final
{
public:
    ScreenshotWriter(std::string filename);
    ~ScreenshotWriter() = default;

    void setScreenshot(bool screenshot_needed = true);
    bool isScreeenshot();
    void doScreeenshot(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::Options> options);

private:
    bool _is_screenshot = false;

    std::string _filename = "";
    std::string _screenshot_path = "";

};

#endif // SCREENSHOT_WRITER_H
