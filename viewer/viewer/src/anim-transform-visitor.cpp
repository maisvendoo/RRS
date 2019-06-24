#include    "anim-transform-visitor.h"
#include    "filesystem.h"
#include    "config-reader.h"
#include    "analog-rotation.h"
#include    "material-animation.h"

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

        if (child->name == "AnalogRotation")
        {
            animation = new AnalogRotation(transform);
            animation->load(cfg);
            return animation;
        }

        if (child->name == "MaterialAnimation")
        {
            for (unsigned int i = 0; i < transform->getNumChildren(); ++i)
            {
                osg::Geode *geode = transform->getChild(i)->asGeode();

                if (geode != nullptr)
                {
                    for (unsigned int j = 0; j < geode->getNumDrawables(); ++j)
                    {
                        osg::Drawable *drawable = geode->getDrawable(j);

                        if (drawable == nullptr)
                            continue;

                        osg::StateSet *stateset = drawable->getOrCreateStateSet();
                        osg::StateAttribute *stateattr = stateset->getAttribute(osg::StateAttribute::MATERIAL);

                        osg::Material *mat = static_cast<osg::Material *>(stateattr);

                        if (mat == nullptr)
                            continue;

                        animation = new MaterialAnimation(mat, drawable);
                        animation->load(cfg);

                        return animation;
                    }
                }
            }
        }
    }

    return nullptr;
}
