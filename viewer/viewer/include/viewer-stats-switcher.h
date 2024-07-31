#ifndef     VIEWER_STATS_SWITCHER_H
#define     VIEWER_STATS_SWITCHER_H

#include    <osgViewer/ViewerEventHandlers>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ViewerStatsHandler : public osgViewer::StatsHandler
{
public:

    ViewerStatsHandler();

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:

    bool is_Shift_L = false;
    bool is_Shift_R = false;
    bool is_Ctrl_L = false;
    bool is_Ctrl_R = false;
    bool is_Alt_L = false;
    bool is_Alt_R = false;
};

#endif // VIEWER_STATS_SWITCHER_H
