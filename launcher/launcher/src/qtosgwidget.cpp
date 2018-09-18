#include    "qtosgwidget.h"
#include    "scancode.h"

#include    <QDebug>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QtOSGWidget::QtOSGWidget(qreal scaleX, qreal scaleY, QWidget *parent)
    : QOpenGLWidget(parent)
    , m_scaleX(scaleX)
    , m_scaleY(scaleY)
    , mGraphicsWindow(new osgViewer::GraphicsWindowEmbedded(this->x(),
                                                            this->y(),
                                                            this->width(),
                                                            this->height()))
    , mViewer(new osgViewer::Viewer)
{
    this->setFocusPolicy(Qt::StrongFocus);

    osg::Camera *camera = new osg::Camera;
    camera->setViewport(0, 0, this->width(), this->height());
    camera->setClearColor(osg::Vec4f(0.9f, 0.9f, 1.0f, 1.0f));
    float aspectRatio = static_cast<float>(this->width()) / static_cast<float>(this->height());
    camera->setProjectionMatrixAsPerspective(30.0f, aspectRatio, 1.0f, 1000.0f);
    camera->setGraphicsContext(mGraphicsWindow);

    mViewer->setCamera(camera);
    mViewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);
    mViewer->realize();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QtOSGWidget::~QtOSGWidget()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgGA::EventQueue *QtOSGWidget::getEventQueue() const
{
    osgGA::EventQueue *eventQueue = mGraphicsWindow->getEventQueue();
    return eventQueue;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::specialKeyPressed(QKeyEvent *event)
{
    quint32 scanCode = event->nativeScanCode();

    switch (scanCode)
    {
    case KEY_L_SHIFT:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Shift_L);
        break;

    case KEY_R_SHIFT:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Shift_R);
        break;

    case KEY_L_CTRL:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Control_L);
        break;

    case KEY_R_CTRL:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Control_R);
        break;

    case KEY_L_ALT:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Alt_L);
        break;

    case KEY_R_ALT:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Alt_R);
        break;

    case KEY_UP:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Up);
        break;

    case KEY_DOWN:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Down);
        break;

    case KEY_LEFT:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Left);
        break;

    case KEY_RIGHT:
        this->getEventQueue()->keyPress(osgGA::GUIEventAdapter::KEY_Right);
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::paintGL()
{
    mViewer->frame();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::resizeGL(int width, int height)
{
    this->getEventQueue()->windowResize(static_cast<int>(this->x() * m_scaleX),
                                        static_cast<int>(this->y() * m_scaleY),
                                        static_cast<int>(width * m_scaleX),
                                        static_cast<int>(height * m_scaleY));

    mGraphicsWindow->resized(static_cast<int>(this->x() * m_scaleX),
                             static_cast<int>(this->y() * m_scaleY),
                             static_cast<int>(width * m_scaleX),
                             static_cast<int>(height * m_scaleY));

    osg::Camera *camera = mViewer->getCamera();

    camera->setViewport(0,
                        0,
                        static_cast<int>(width * m_scaleX),
                        static_cast<int>(height * m_scaleY));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::initializeGL()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool QtOSGWidget::event(QEvent *e)
{
    bool handled = QOpenGLWidget::event(e);
    this->update();
    return handled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::keyPressEvent(QKeyEvent *event)
{
    keyboard.setKey(event->key());

    int c = *event->text().toLocal8Bit().data();

    if (c == 0)
        specialKeyPressed(event);
    else
        this->getEventQueue()->keyPress((osgGA::GUIEventAdapter::KeySymbol) *(event->text().toLocal8Bit().data()));

    emit sendDataToSimulator(keyboard.serialize());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::keyReleaseEvent(QKeyEvent *event)
{
    keyboard.resetKey(event->key());

    emit sendDataToSimulator(keyboard.serialize());
}
