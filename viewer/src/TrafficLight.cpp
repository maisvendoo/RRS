#include "TrafficLight.h"

#include "filesystem.h"
#include "AnimatedPagedLOD.h"
#include "ProcAnimation.h"
#include "Logger.h"

#include <QDataStream>

#include <algorithm>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLight::TrafficLight()
{
    server_signals.resize(lens_state.size());
    std::fill(server_signals.begin(), server_signals.end(), 0.0f);

    std::fill(lens_state.begin(), lens_state.end(), false);
    old_lens_state = lens_state;

    animated_pagedLOD = AnimatedPagedLOD::create();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::step(float t, float dt)
{
    if (transform->children.empty() ||
        (!animated_pagedLOD->children[0].node) ||
        animated_pagedLOD->animations_map->animations.empty())
    {
        return;
    }

    bool changed = (old_lens_state != lens_state);
    if (changed)
    {
        old_lens_state = lens_state;

        for (size_t i = 0; i < lens_state.size(); ++i)
        {
            server_signals[i] = static_cast<float>(lens_state[i]);
        }
    }

    for (const auto& [signal_id, animation] : animated_pagedLOD->animations_map->animations)
    {
        animation->setSignals(&(server_signals));
        animation->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::deserialize(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);

    stream >> connector_name;
    stream >> signal_dir;
    stream >> letter;
    stream >> signal_model;

    for (bool& lens : lens_state)
    {
        stream >> lens;
    }

    stream >> position.x >> position.y >> position.z;
    stream >> orth.x >> orth.y >> orth.z;
    stream >> right.x >> right.y >> right.z;
    stream >> up.x >> up.y >> up.z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getConnectorName() const noexcept
{
    return connector_name;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::int8_t TrafficLight::getSignalDirection() const noexcept
{
    return signal_dir;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getLetter() const noexcept
{
    return letter;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrafficLight::getModelName() const noexcept
{
    return signal_model;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const lens_state_t& TrafficLight::getLensState() const noexcept
{
    return lens_state;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrafficLight::loadSignal(std::string &models_dir_path,
                              std::string &animations_dir,
                              vsg::ref_ptr<vsg::Options> options)
{
    if (signal_model.isEmpty() || (signal_model == "empty_line"))
    {
        return false;
    }

    FileSystem& fs = FileSystem::getInstance();
    std::string model_filename_path = fs.combinePath(models_dir_path, signal_model.toStdString());
    model_filename_path += ".gltf";

    if (!vsg::fileExists(model_filename_path))
    {
        LOG_WARN("Fail to find file: %s", model_filename_path.c_str());
        return false;
    }

    vsg::dmat4 m1 = vsg::translate(position);

    vsg::dmat4 m2{
        right.x,right.y,right.z,0.0,
        orth.x, orth.y, orth.z, 0.0,
        up.x,   up.y,   up.z,   0.0,
        0.0,    0.0,    0.0,    1.0};

    transform->matrix = m1 * m2;

    // Load model
    animated_pagedLOD->animations_dir = animations_dir;
    animated_pagedLOD->filename = model_filename_path;
    animated_pagedLOD->bound = vsg::dsphere(vsg::dvec3(0.0, 0.0, 0.0), 50.0);
    animated_pagedLOD->children[0] = vsg::PagedLOD::Child{0.0, {}};
    animated_pagedLOD->options = options;
    transform->addChild(animated_pagedLOD);

    return true;
}
