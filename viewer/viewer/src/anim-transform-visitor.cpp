#include    "anim-transform-visitor.h"
#include    "filesystem.h"
#include    "config-reader.h"
#include    "analog-rotation.h"
#include    "analog-translation.h"
#include    "material-animation.h"

#include    "material-animation-visitor.h"
#include    "material-rgb-animation-visitor.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
AnimTransformVisitor::AnimTransformVisitor(animations_t *animations,
                                           const std::string &vehicle_config)
    : osg::NodeVisitor ()
    , animations(animations)
    , vehicle_config(vehicle_config)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimTransformVisitor::apply(osg::Transform &transform)
{
    osg::MatrixTransform *matrix_trans = static_cast<osg::MatrixTransform *>(&transform);
    std::string name = matrix_trans->getName();

    ProcAnimation *animation = create_animation(name, matrix_trans);

    if (animation != nullptr)
    {
        animation->setName(name);
        animations->insert(animation->getSignalID(), animation);
    }

    traverse(transform);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcAnimation *AnimTransformVisitor::create_animation(const std::string &name,
                                                      osg::MatrixTransform *transform)
{
    FileSystem &fs = FileSystem::getInstance();
    std::string data_dir = fs.getDataDir();
    std::string file_path = data_dir + fs.separator()
            + "animations" + fs.separator()
            + vehicle_config + fs.separator()
            + name + ".xml";

    ConfigReader cfg(file_path);

    if (!cfg.isOpenned())
    {
        return nullptr;
    }

    osgDB::XmlNode *rootNode = cfg.getConfigNode();


    for (auto it = rootNode->children.begin(); it != rootNode->children.end(); ++it)
    {
        osgDB::XmlNode  *child = *it;

        ProcAnimation *animation = nullptr;

        // Анимация поворота (в том числе и бесконечного)
        if (child->name == "AnalogRotation")
        {
            animation = new AnalogRotation(transform);
            animation->load(cfg);
            return animation;
        }

        // Анимация перемещения
        if (child->name == "AnalogTranslation")
        {
            animation = new AnalogTranslation(transform);
            animation->load(cfg);
            return animation;
        }

        // Анимация материала
        if (child->name == "MaterialAnimation")
        {
            MaterialAnimationVisitor mav(animations, &cfg);
            mav.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

            transform->accept(mav);
        }

        // RGB-анимация материала
        if (child->name == "MaterialRGBAnimation")
        {
            MaterialRGBAnimationVisitor mav(animations, &cfg);
            mav.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

            transform->accept(mav);
        }
    }

    return nullptr;
}
