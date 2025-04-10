#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "animations-list.h"
#include "signal-types.h"

#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Node.h>

#include <QString>

#include <string>

class QByteArray;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLight
{
public:
    TrafficLight();

    void deserialize(QByteArray& data);

    const QString& getConnectorName() const;
    int getSignalDirection() const;
    const QString& getLetter() const;
    const QString& getModelName() const;

    const vsg::dvec3& getPosition() const;
    const vsg::dvec3& getOrth() const;
    const vsg::dvec3& getRight() const;
    const vsg::dvec3& getUp() const;

    void set_node(vsg::ref_ptr<vsg::Node> node);

    void load_animations(const std::string& animations_dir, vsg::ref_ptr<vsg::Options> options, vsg::ref_ptr<vsg::PropagateDynamicObjects> pdo, vsg::ref_ptr<vsg::Duplicate> duplicate);

    void step(float t, float dt);

private:
    vsg::ref_ptr<vsg::Node> node;

    QString connector_name = "";
    int signal_dir = 0;
    bool is_busy = false;
    QString letter = "";
    QString signal_model = "";

    lens_state_t lens_state;
    lens_state_t old_lens_state;

    vsg::dvec3 pos;
    vsg::dvec3 orth;
    vsg::dvec3 right;
    vsg::dvec3 up;

    animations_t animations;
};

#endif // TRAFFIC_LIGHT_H
