#ifndef     MAIN_DIALOG_WINDOW_H
#define     MAIN_DIALOG_WINDOW_H

#include    <osgViewer/Viewer>
#include    <osgWidget/WindowManager>
#include    <osgWidget/Box>
#include    <osgWidget/Label>

class MainDialogWindow
{
public:

    MainDialogWindow(osgViewer::Viewer *viewer);
    ~MainDialogWindow();

    osg::ref_ptr<osgWidget::WindowManager> wm;

private:


};

#endif // MAINDIALOGWINDOW_H
