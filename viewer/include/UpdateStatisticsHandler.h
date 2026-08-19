#pragma once
#ifndef UPDATE_STATISTICS_HANDLER_H
#define UPDATE_STATISTICS_HANDLER_H

#include <vsg/core/Visitor.h>
#include <vsg/core/Inherit.h>
#include <chrono>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class UpdateStatisticsHandler final : public vsg::Inherit<vsg::Visitor, UpdateStatisticsHandler>
{
public:
    UpdateStatisticsHandler();

    void apply(vsg::FrameEvent& frame) override;

    double getAverageFPS() const noexcept;
    double getLowestFPS() const noexcept;

private:

    double average_FPS = 0.0;
    double lowest_FPS = 0.0;

    static constexpr uint8_t FRAME_ARRAY_SIZE = 250;
    uint8_t frame_index = 0;
    std::array<std::chrono::steady_clock::time_point, FRAME_ARRAY_SIZE> frames_timepoints;
    std::array<double, FRAME_ARRAY_SIZE> frames_FPS;
};

#endif // UPDATE_STATISTICS_HANDLER_H
