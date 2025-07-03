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
    explicit ScreenshotWriter(const std::string& filename);
    ~ScreenshotWriter() noexcept = default;

    void setScreenshot(bool screenshot_needed = true) noexcept;
    bool isScreeenshot() const noexcept;
    void doScreeenshot(vsg::ref_ptr<vsg::Window> window, vsg::ref_ptr<vsg::Options> options);

private:
    bool _is_screenshot = false;

    std::string _filename = "";
    std::string _screenshot_path = "";

};

#endif // SCREENSHOT_WRITER_H
