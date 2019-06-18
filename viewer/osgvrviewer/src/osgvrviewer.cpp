#include    "osgvrviewer.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GraphicsWindowViewer::GraphicsWindowViewer(osg::ArgumentParser &arguments,
                                           osgViewer::GraphicsWindow *graphicsWindow)

    : osgViewer::Viewer(arguments), _graphicsWindow(graphicsWindow)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GraphicsWindowViewer::eventTraversal()
{
    if (_graphicsWindow.valid() && _graphicsWindow->checkEvents())
    {
        osgGA::EventQueue::Events events;
        _graphicsWindow->getEventQueue()->copyEvents(events);
        osgGA::EventQueue::Events::iterator itr;
        for (itr = events.begin(); itr != events.end(); ++itr)
        {
            osgGA::GUIEventAdapter* event = dynamic_cast<osgGA::GUIEventAdapter*>(itr->get());
            if (event != nullptr && event->getEventType() == osgGA::GUIEventAdapter::CLOSE_WINDOW)
            {
                // We have "peeked" at the event queue for the GraphicsWindow and
                // found a CLOSE_WINDOW event. This would normally mean that OSG
                // is about to release OpenGL contexts attached to the window.
                // For some applications it is better to make the application
                // terminate in a more "normal" way e.g. as it does when <Esc>
                // key had been pressed.
                // In this way we can safely perform any cleanup required after
                // termination of the Viewer::run() loop, i.e. cleanup that would
                // otherwise malfunction if the earlier processing of the CLOSE_WINDOW
                // event had already released required OpenGL contexts.
                // So, here we "get in early" and remove the close event and append
                // a quit application event.
                events.erase(itr);
                _graphicsWindow->getEventQueue()->setEvents(events);
                _graphicsWindow->getEventQueue()->quitApplication();
                break;
            }
        }
    }
    osgViewer::Viewer::eventTraversal();
}
