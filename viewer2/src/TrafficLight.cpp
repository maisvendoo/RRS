#include "TrafficLight.h"

#include "filesystem.h"
#include "ProcAnimation.h"
#include "LoadModelOperation.h"
// #include "MyGui.h"

#include <vsg/threading/OperationThreads.h>

#include <QBuffer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLight::TrafficLight()
{
    std::fill(lens_state.begin(), lens_state.end(), false);
    old_lens_state = lens_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::step(float t, float dt)
{
    if (transform->children.empty() || animations->animations.empty())
    {
        return;
    }

    bool changed = (old_lens_state != lens_state);
    if (changed)
    {
        old_lens_state = lens_state;
    }

    for (auto& [signal_id, animation] : animations->animations)
    {
        if (changed)
        {
            animation->setPosition(lens_state[signal_id]);
        }

        animation->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::deserialize(QByteArray& data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    stream >> connector_name;
    stream >> signal_dir;
    stream >> is_busy;
    stream >> letter;
    stream >> signal_model;

    for (std::size_t i = 0; i < lens_state.size(); ++i)
    {
        stream >> lens_state[i];
    }

    stream >> position.x >> position.y >> position.z;
    stream >> orth.x >> orth.y >> orth.z;
    stream >> right.x >> right.y >> right.z;
    stream >> up.x >> up.y >> up.z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getConnectorName() const
{
    return connector_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int TrafficLight::getSignalDirection() const
{
    return signal_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getLetter() const
{
    return letter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getModelName() const
{
    return signal_model;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrafficLight::loadSignal(std::string &models_dir_path,
                              std::string &animations_dir,
                              vsg::ref_ptr<vsg::Viewer> viewer,
                              vsg::ref_ptr<vsg::Options> options)
{
    if (signal_model.isEmpty())
        return false;

    vsg::dmat4 m1 = vsg::translate(position);

    vsg::dmat4 m2(
        right.x,    -orth.x,    up.x,   0,
        -right.y,   orth.y,     up.y,   0,
        right.z,    orth.z,     up.z,   0,
        0,          0,          0,      1
    );

    transform->matrix = m1 * m2;

    FileSystem& fs = FileSystem::getInstance();
    std::string model_filename_path = fs.combinePath(models_dir_path, signal_model.toStdString());
    model_filename_path += ".gltf";
    std::string textures_dir = "";

    // Load model
    options->operationThreads->add(LoadModelOperation::create(viewer,
                                                              transform,
                                                              model_filename_path,
                                                              animations_dir,
                                                              options,
                                                              animations));
//    GUIParams::nodes.emplace_back(transform);
    return true;
}
