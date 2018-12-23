//------------------------------------------------------------------------------
//
//      HUD (Head Up Display)
//      (c) maisvendoo, 21/12/2018
//
//------------------------------------------------------------------------------

#ifndef     HUD_H
#define     HUD_H

#include    <QObject>

#include    <osg/Camera>
#include    <osgText/Font>
#include    <osgText/Text>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class HUD : public QObject
{
    Q_OBJECT

public:

    HUD(QObject *parent = Q_NULLPTR);

    ~HUD();

    void init(double left, double right, double bottom, double top);

private:

    osg::ref_ptr<osg::Camera>   hud_camera;
};

#endif // HUD_H
