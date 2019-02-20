#ifndef     HUD_H
#define     HUD_H

#include    <osg/Camera>
#include    <osg/Switch>
#include    <osgText/Text>
#include    <osgViewer/View>
#include    <osgGA/GUIEventHandler>

#include    <QObject>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class HUD : public QObject
{
    Q_OBJECT

public:

    HUD(int width, int height, QObject *parent = Q_NULLPTR);

    ~HUD();

    osg::Camera *getCamera();

    osg::Switch *getScene();

    osgViewer::View *getView();

private:

    osg::ref_ptr<osg::Camera>   camera;
    osg::ref_ptr<osg::Switch>   scene;
    std::string                 fontPath;
    osg::ref_ptr<osgViewer::View> view;

    osg::Camera *createCamera(int width, int height);
    osgText::Text *createText(const osg::Vec3 &position, std::wstring text, float size);
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class KeyboardHUDHandler : public osgGA::EventHandler
{
public:

    KeyboardHUDHandler(osg::Switch *switchNode);

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

private:

    osg::Switch *switchNode;
};

#endif // HUD_H
