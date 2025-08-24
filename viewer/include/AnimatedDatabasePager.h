#ifndef ANIMATED_DATABASE_PAGER_H
#define ANIMATED_DATABASE_PAGER_H

#include <vsg/io/DatabasePager.h>

class animations_t;
class AnimatedPagedLOD;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AnimatedDatabasePager : public vsg::Inherit<vsg::DatabasePager, AnimatedDatabasePager>
{
public:
    AnimatedDatabasePager() = default;

    void start(uint32_t numReadThreads = 4) override;

protected:
    ~AnimatedDatabasePager() = default;

private:

    vsg::ref_ptr<vsg::Node> loadAnimations(vsg::ref_ptr<AnimatedPagedLOD> aplod,
                                           vsg::ref_ptr<vsg::Node> node);
};

#endif // ANIMATED_DATABASE_PAGER_H
