#include "UpdateStatisticsHandler.h"
#include "vsg/ui/ApplicationEvent.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
UpdateStatisticsHandler::UpdateStatisticsHandler()
{
    std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
    std::fill(frames_timepoints.begin(), frames_timepoints.end(), t);
    std::fill(frames_FPS.begin(), frames_FPS.end(), 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void UpdateStatisticsHandler::apply(vsg::FrameEvent& frame)
{
    uint8_t prev_index = frame_index;
    ++frame_index;
    if (frame_index >= FRAME_ARRAY_SIZE)
        frame_index = 0;

    double frames_count = std::min(frame.frameStamp->frameCount, uint64_t(FRAME_ARRAY_SIZE));
    double frames_duration = std::chrono::duration<double, std::chrono::seconds::period>
                             (frame.frameStamp->time - frames_timepoints[frame_index]).count();
    frames_timepoints[frame_index] = frame.frameStamp->time;
    if (frames_duration < 1e-10)
        return;

    average_FPS = frames_count / frames_duration;

    double last_frame_duration = std::chrono::duration<double, std::chrono::seconds::period>
                                 (frames_timepoints[frame_index] - frames_timepoints[prev_index]).count();
    if (last_frame_duration < 1e-10)
        return;

    frames_FPS[frame_index] = 1.0 / last_frame_duration;
    lowest_FPS = *std::min_element(frames_FPS.begin(), frames_FPS.end());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double UpdateStatisticsHandler::getAverageFPS() const
{
    return average_FPS;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double UpdateStatisticsHandler::getLowestFPS() const
{
    return lowest_FPS;
}
