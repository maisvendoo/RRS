#include "FindModelAnimations.h"

#include "animations-list.h"
#include "CfgReader.h"
#include "filesystem.h"
#include "ProcModelAnimation.h"

#include <vsg/animation/FindAnimations.h>

#include <QDomNode>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FindModelAnimations::FindModelAnimations(const FindModelAnimationsCreateInfo& create_info)
    : node(create_info.node)
    , animations(create_info.animations)
{
    FileSystem& fs = FileSystem::getInstance();
    std::string animations_dir_path = fs.getDataDir() + fs.separator()
                                      + "animations" + fs.separator()
                                      + create_info.animations_dir + fs.separator();
    animations_dir = animations_dir_path;

    find_animations();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FindModelAnimations::find_animations()
{
    // Собираем все имеющиеся внутри модели анимации
    vsg::FindAnimations findAnimations;
    node->accept(findAnimations);

    for (auto& model_animation : findAnimations.animations)
    {
        // Пытаемся подключить анимацию к сигналу из движка
        vsg::ref_ptr<ProcAnimation> animation = create_animation(model_animation);
        if (animation)
        {
            animation->name = model_animation->name;
            animations->thread_safe_insert({animation->getSignalID(), animation});
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<ProcAnimation> FindModelAnimations::create_animation(vsg::ref_ptr<vsg::Animation> model_animation)
{
    // Пытаемся найти конфиг к анимации
    std::string file_path = animations_dir + model_animation->name + ".xml";

    CfgReader cfg;
    if (cfg.load(file_path.c_str()))
    {
        QDomNode config_section = cfg.getFirstSection("ModelAnimation");
        if (!config_section.isNull())
        {
            // Создаём и настраиваем управление анимацией по сигналу из движка
            vsg::ref_ptr<ProcAnimation> animation = ProcModelAnimation::create(model_animation);
            animation->load(cfg);
            return animation;
        }
    }

    return nullptr;
}
