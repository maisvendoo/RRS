#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "animations-list.h"
#include "signal-types.h"

#include <vsg/core/Object.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/Options.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/MatrixTransform.h>

#include <QString>

#include <string>

class QByteArray;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLight
{
public:

    vsg::ref_ptr<vsg::MatrixTransform> transform = vsg::MatrixTransform::create();
    vsg::dvec3  position = vsg::dvec3(0.0, 0.0, 0.0);
    vsg::dvec3  orth = vsg::dvec3(0.0, 1.0, 0.0);
    vsg::dvec3  up = vsg::dvec3(0.0, 0.0, 1.0);
    vsg::dvec3  right = vsg::dvec3(1.0, 0.0, 0.0);

    TrafficLight();

    void step(float t, float dt);

    void deserialize(QByteArray& data);

    const QString& getConnectorName() const;
    int getSignalDirection() const;
    const QString& getLetter() const;
    const QString& getModelName() const;
/*
    const vsg::dvec3& getPosition() const;
    const vsg::dvec3& getOrth() const;
    const vsg::dvec3& getRight() const;
    const vsg::dvec3& getUp() const;
*/
    bool loadSignal(std::string& models_dir_path,
                    std::string& animations_dir,
                    vsg::ref_ptr<vsg::Viewer> viewer,
                    vsg::ref_ptr<vsg::Options> options);

private:
//    vsg::ref_ptr<vsg::Node> node;

    QString connector_name = "";
    int signal_dir = 0;
    bool is_busy = false;
    QString letter = "";
    QString signal_model = "";

    lens_state_t lens_state;
    lens_state_t old_lens_state;

    animations_t animations = {};
};

#endif // TRAFFIC_LIGHT_H
