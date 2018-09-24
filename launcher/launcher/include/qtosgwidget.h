//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class QtOSGWidget : public QOpenGLWidget
{
    Q_OBJECT

public:

    QtOSGWidget(qreal scaleX, qreal scaleY, QWidget *parent = Q_NULLPTR);

    virtual ~QtOSGWidget();

private:

    qreal   m_scaleX;
    qreal   m_scaleY;

    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> mGraphicsWindow;
    osg::ref_ptr<osgViewer::Viewer> mViewer;

    Keyboard    keyboard;

    osgGA::EventQueue *getEventQueue() const;

    void specialKeyPressed(QKeyEvent *event);

signals:

    void sendDataToSimulator(QByteArray data);

protected:

    virtual void paintGL();

    virtual void resizeGL(int width, int height);

    virtual void initializeGL();

    virtual bool event(QEvent *e);

    virtual void keyPressEvent(QKeyEvent *event);

    virtual void keyReleaseEvent(QKeyEvent *event);

public slots:

    void getDataFromSimulator(QByteArray data);
};

#endif // QT_OSG_WIDGET_H
