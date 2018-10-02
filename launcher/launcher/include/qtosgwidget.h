//------------------------------------------------------------------------------
//
//      OSG integration in Qt application
//      (c) mainvendoo, 18/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief OSG integration in Qt application
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 18/09/2018
 */

#ifndef     QT_OSG_WIDGET_H
#define     QT_OSG_WIDGET_H

#include    <QOpenGLWidget>
#include    <QMouseEvent>
#include    <QWheelEvent>

#include    <osg/ref_ptr>
#include    <osgViewer/GraphicsWindow>
#include    <osgViewer/Viewer>
#include    <osg/ShapeDrawable>
#include    <osg/StateSet>
#include    <osg/Material>

#include    <osgGA/EventQueue>
#include    <osgGA/TrackballManipulator>

#include    "keyboard.h"
#include    "route-loader.h"
#include    "filesystem.h"

/*!
 * \class
 * \brief OSG widget
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class QtOSGWidget : public QOpenGLWidget
{
    Q_OBJECT

public:

    /// Constructor
    QtOSGWidget(FileSystem *fs, qreal scaleX, qreal scaleY, QWidget *parent = Q_NULLPTR);
    /// Destructor
    virtual ~QtOSGWidget();

    void setFileSystem(FileSystem *fs);

private:

    ///
    FileSystem  *fs;

    /// Widget x scale
    qreal   m_scaleX;
    /// Widget y scale
    qreal   m_scaleY;

    /// OSG graphic window
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> mGraphicsWindow;
    /// OSG viewer
    osg::ref_ptr<osgViewer::Viewer> mViewer;

    /// Keyboard processor
    Keyboard    keyboard;

    /// Get event from OSG queue
    osgGA::EventQueue *getEventQueue() const;

    /// Parse special key (as Shift, Control, Alt etc.) pressing
    void specialKeyPressed(QKeyEvent *event);

signals:

    /// Send data to train simulator
    void sendDataToSimulator(QByteArray data);
    /// Start train simulation
    void startSimulation();
    /// Stop train simulation
    void stopSimulation();

protected:

    virtual void paintGL();

    virtual void resizeGL(int width, int height);

    virtual void initializeGL();

    virtual bool event(QEvent *e);

    virtual void keyPressEvent(QKeyEvent *event);

    virtual void keyReleaseEvent(QKeyEvent *event);

    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void mousePressEvent(QMouseEvent *event);

    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void wheelEvent(QWheelEvent *event);

public slots:

    /// Get data from train simulator
    void getDataFromSimulator(QByteArray data);

private:

    unsigned int getMouseButtonOSG(QMouseEvent *event);
};

#endif // QT_OSG_WIDGET_H
