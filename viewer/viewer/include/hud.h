#ifndef     HUD_H
#define     HUD_H

#include    <osg/Camera>
#include    <osg/Switch>

#include    <QObject>

class HUD : public QObject
{
    Q_OBJECT

public:

    HUD(int width, int height, QObject *parent = Q_NULLPTR);

    ~HUD();

    osg::Camera *getCamera();

    osg::Switch *getScene();

private:

    osg::ref_ptr<osg::Camera> camera;
     osg::ref_ptr<osg::Switch> scene;

    osg::Camera *createCamera(int width, int height);
};

#endif // HUD_H
