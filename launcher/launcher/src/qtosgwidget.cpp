#include    "qtosgwidget.h"
#include    "scancode.h"

#include    <QDebug>
#include    <osgDB/ReadFile>
#include    <osg/PositionAttitudeTransform>
#include    <osgUtil/Optimizer>
#include    <osg/BlendFunc>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QtOSGWidget::QtOSGWidget(FileSystem *fs, qreal scaleX, qreal scaleY, QWidget *parent)
    : QOpenGLWidget(parent)
    , fs(fs)
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

    RouteLoader *loader = initRouteLoader(fs->combinePath(fs->getLibDirectory(), "zds-route-loader"));
    loader->setFileSystem(fs);
    osg::Group *root = loader->load("konotop");

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addChild(root);

    mViewer->setCamera(camera);
    mViewer->setSceneData(geode);

    osgGA::TrackballManipulator *manipulator = new osgGA::TrackballManipulator;
    manipulator->setAllowThrow(false);
    this->setMouseTracking(true);
    mViewer->setCameraManipulator(manipulator);

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
void QtOSGWidget::setFileSystem(FileSystem *fs)
{
    this->fs = fs;
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
    osg::Geode *geode = dynamic_cast<osg::Geode *>(mViewer->getSceneData());
    osg::StateSet *stateSet = geode->getOrCreateStateSet();
    osg::Material *mat = new osg::Material;
    mat->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
    stateSet->setAttributeAndModes(mat, osg::StateAttribute::ON);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
    stateSet->setAttributeAndModes(bf);
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
    if (event->key() == Qt::Key_S)
        emit startSimulation();

    if (event->key() == Qt::Key_X)
        emit stopSimulation();    

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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::mouseMoveEvent(QMouseEvent *event)
{
    this->getEventQueue()->mouseMotion(event->x() * m_scaleX,
                                       event->y() * m_scaleY);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::mousePressEvent(QMouseEvent *event)
{
    this->getEventQueue()->mouseButtonPress(event->x() * m_scaleX,
                                            event->y() * m_scaleY,
                                            this->getMouseButtonOSG(event));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::mouseReleaseEvent(QMouseEvent *event)
{
    this->getEventQueue()->mouseButtonRelease(event->x() * m_scaleX,
                                              event->y() * m_scaleY,
                                              this->getMouseButtonOSG(event));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::wheelEvent(QWheelEvent *event)
{
    int delta = event->delta();

    osgGA::GUIEventAdapter::ScrollingMotion motion = delta > 0 ?
                osgGA::GUIEventAdapter::SCROLL_UP :
                osgGA::GUIEventAdapter::SCROLL_DOWN;

    this->getEventQueue()->mouseScroll(motion);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void QtOSGWidget::getDataFromSimulator(QByteArray data)
{
    (void) data;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
unsigned int QtOSGWidget::getMouseButtonOSG(QMouseEvent *event)
{
    unsigned int button = 0;

    switch (event->button())
    {
    case Qt::LeftButton:
        button = 1;
        break;
    case Qt::MiddleButton:
        button = 2;
        break;
    case Qt::RightButton:
        button = 3;
        break;

    default:
        break;
    }

    return button;
}
