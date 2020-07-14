#include    "proc-animation.h"
#include    "get-value.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcAnimation::ProcAnimation(osg::MatrixTransform *transform)
    : pos(0.0f)
    , duration(0.0f)
    , signal_id(0)
    , transform(transform)
    , name("")
    , is_fixed_signal(false)
    , fixed_signal(0.0f)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcAnimation::~ProcAnimation()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcAnimation::step(float t, float dt)
{
    anim_step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcAnimation::setName(const std::string &name)
{
    this->name = name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string ProcAnimation::getName() const
{
    return name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcAnimation::load(const std::string &path)
{
    ConfigReader cfg(path);

    if (!cfg.isOpenned())
    {
        OSG_WARN << "WARNING: Animation file " + path + " is't found" << std::endl;
        return false;
    }

    return load_config(cfg);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcAnimation::load(ConfigReader &cfg)
{
    loadKeyPoints(cfg);

    return load_config(cfg);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void ProcAnimation::setPosition(float pos)
{
    if (is_fixed_signal)
        this->pos = fixed_signal;
    else
        this->pos = pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
size_t ProcAnimation::getSignalID() const
{
    return signal_id;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ProcAnimation::loadKeyPoints(ConfigReader &cfg)
{
    osgDB::XmlNode *config_node = cfg.getConfigNode();

    if (config_node == nullptr)
    {
        OSG_FATAL << "There is no Config node in file" << std::endl;
        return false;
    }

    for (auto it = config_node->children.begin(); it != config_node->children.end(); ++it)
    {
        osgDB::XmlNode *child = *it;

        if (child->name == "KeyPoint")
        {
            key_point_t key_point;

            osgDB::XmlNode *node = nullptr;

            node = cfg.findSection(child, "Param");

            if (node == nullptr)
            {
                OSG_FATAL << "Param of keypoint is missing" << std::endl;
                continue;
            }

            getValue(node->contents, key_point.param);

            node = cfg.findSection(child, "Value");

            if (node == nullptr)
            {
                OSG_FATAL << "Value of keypoint is missing" << std::endl;
                continue;
            }

            getValue(node->contents, key_point.value);

            keypoints.push_back(key_point);
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float ProcAnimation::interpolate(float param)
{
    if (keypoints.size() <= 1)
        return 0.0f;

    size_t next_idx = 0;
    key_point_t begin_point = findBeginKeyPoint(param, next_idx);
    key_point_t next_point = keypoints.at(next_idx);

    float range = next_point.param - begin_point.param;

    if (range < 1e-6f)
        return 0.0f;

    float value = begin_point.value + (param - begin_point.param) * (next_point.value - begin_point.value) / range;

    return value;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ProcAnimation::key_point_t ProcAnimation::findBeginKeyPoint(float param, size_t &next_idx)
{
    key_point_t key_point;

    if (keypoints.size() == 0)
        return key_point;

    size_t left_idx = 0;
    size_t right_idx = keypoints.size() - 1;
    size_t idx = (left_idx + right_idx) / 2;

    while (idx != left_idx)
    {
        key_point = keypoints.at(idx);

        if (param <= key_point.param)
            right_idx = idx;
        else
            left_idx = idx;

        idx = (left_idx + right_idx) / 2;
    }

    key_point = keypoints.at(idx);
    next_idx = idx + 1;

    return key_point;
}
