#include    "hud.h"
#include    "filesystem.h"

#include    <osg/Geode>

#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HUD::HUD(int width, int height, QObject *parent) : QObject (parent)
  , camera(nullptr)
  , scene(new osg::Switch)
  , view(new osgViewer::View)
  , statusBar(nullptr)
{
    FileSystem &fs = FileSystem::getInstance();
    fontPath = fs.getFontsDir() + fs.separator() + "dejavu-sans-mono.ttf";

    camera = createCamera(width, height);    
    camera->setViewport(0, 0, width, height);
    view->setCamera(camera.get());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
HUD::~HUD()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Camera *HUD::getCamera()
{
    return camera.get();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Switch *HUD::getScene()
{
    return scene.get();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgViewer::View *HUD::getView()
{
    return view.get();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void HUD::setStatusBar(const std::wstring &msg)
{
    //std::cout << qPrintable(msg) << std::endl;

    if (statusBar.valid())
        statusBar->setText(msg.c_str());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osg::Camera *HUD::createCamera(int width, int height)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    camera->setAllowEventFocus(false);

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    statusBar = createText(osg::Vec3(5, 66, 0), L"", 15.0f);

    geode->addDrawable(statusBar.get());

    scene->addChild(geode.get(), false);
    camera->addChild(scene.get());

    return camera.release();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
osgText::Text *HUD::createText(const osg::Vec3 &position,
                               std::wstring content,
                               float size,
                               const osg::Vec4 &color)
{
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setFont(fontPath);
    text->setCharacterSize(size);
    text->setPosition(position);
    text->setText(content.c_str());    
    text->setColor(color);

    return text.release();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeyboardHUDHandler::KeyboardHUDHandler(osg::Switch *switchNode)
    : switchNode(switchNode)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool KeyboardHUDHandler::handle(const osgGA::GUIEventAdapter &ea,
                                osgGA::GUIActionAdapter &aa)
{
    Q_UNUSED(aa)

    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:

        switch (ea.getKey())
        {
        case osgGA::GUIEventAdapter::KEY_F1:
            bool state = switchNode->getValue(0);
            state = !state;
            switchNode->setValue(0, state);
            break;
        }

        break;

    default:

        break;
    }

    return false;
}
