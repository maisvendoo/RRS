#include    "MainDialogWindow.h"

MainDialogWindow::MainDialogWindow(osgViewer::Viewer *viewer)
{
    wm = new osgWidget::WindowManager(
                viewer,
                1360,
                768,
                0xF0000000,
                osgWidget::WindowManager::WM_PICK_DEBUG);

    osgWidget::Label *label = new osgWidget::Label("", "");
    label->setFontSize(13);
    label->setFontColor(0.0f, 0.0f, 0.0f, 1.0f);
    label->setLabel("Ho-ho-ho!");

    wm->addChild(label);

    wm->resizeAllWindows();
}

MainDialogWindow::~MainDialogWindow()
{

}
