#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "animations-list.h"
#include "signal-types.h"
#include <qcontainerfwd.h>
#include <QString>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Node.h>



class TrafficLight
{
public:
    TrafficLight();

    void deserialize(QByteArray& data);

    const QString& getConnectorName() const;
    int getSignalDirection() const;
    const QString& getLetter() const;
    const QString& getModelName() const;
    const vsg::vec3& getPosition() const;
    const vsg::vec3& getOrth() const;
    const vsg::vec3& getRight() const;
    const vsg::vec3& getUp() const;

    void setNode(vsg::ref_ptr<vsg::Node> node);

    void load_animations(const std::string& animations_dir);

private:
    QString connector_name = "";
    int signal_dir = 0;
    bool is_busy = false;
    QString letter = "";
    QString signal_model = "";
    lens_state_t lens_state;
    lens_state_t old_lens_state;
    vsg::vec3 pos;
    vsg::vec3 orth;
    vsg::vec3 right;
    vsg::vec3 up;

    vsg::ref_ptr<vsg::Node> node;
    animations_t animations;
};

#endif // TRAFFIC_LIGHT_H
