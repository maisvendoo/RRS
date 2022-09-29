#ifndef     VIEWER_FRAME_THREAD_H
#define     VIEWER_FRAME_THREAD_H

#include    <osgViewer/ViewerBase>
#include    <QApplication>

class ViewerFrameThread : public OpenThreads::Thread
{
public:

    ViewerFrameThread(osgViewer::ViewerBase *viewerBase, bool do_exit = true)
        : viewerBase(viewerBase)
        , do_exit(do_exit)
    {

    }

    ~ViewerFrameThread()
    {
        cancel();

        while (isRunning())
        {
            OpenThreads::Thread::YieldCurrentThread();
        }
    }

    int cancel() override
    {
        viewerBase->setDone(true);
        return 0;
    }

    void run() override
    {
        int result = viewerBase->run();

        if (do_exit)
            QApplication::exit(result);
    }

private:

    osg::ref_ptr<osgViewer::ViewerBase> viewerBase;
    bool do_exit;
};

#endif // VIEWER_FRAME_THREAD_H
