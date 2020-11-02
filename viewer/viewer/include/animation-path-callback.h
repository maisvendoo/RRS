#ifndef     ANIMATION_PATH_CALLBACK_H
#define     ANIMATION_PATH_CALLBACK_H

#include    <osg/AnimationPath>

class AnimationPathCallback : public osg::AnimationPathCallback
{
public:

    AnimationPathCallback() : osg::AnimationPathCallback()
    {

    }

    void operator() (osg::Node *node, osg::NodeVisitor *nv)
    {
        if (getPause())
        {
            return;
        }

        setPause(true);
        osg::AnimationPathCallback::operator()(node, nv);
    }

private:


};

#endif // ANIMATION_PATH_CALLBACK_H
