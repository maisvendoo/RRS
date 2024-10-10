#ifndef     TRAFFIC_LIGHTS_HANDLER_H
#define     TRAFFIC_LIGHTS_HANDLER_H

#include    <QObject>
#include    <QMap>

#include    <osgGA/GUIEventHandler>
#include    <osg/PagedLOD>

#include    <traffic-light.h>
#include    <settings.h>
#include    <animation-manager.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrafficLightsHandler : public QObject, public osgGA::GUIEventHandler
{
    Q_OBJECT

public:

    TrafficLightsHandler(QObject *parent = Q_NULLPTR);

    ~TrafficLightsHandler();

    virtual bool handle(const osgGA::GUIEventAdapter &ea,
                        osgGA::GUIActionAdapter &aa);

    void deserialize(QByteArray &data);

    void load_signal_models(const settings_t &settings);

    osg::Group *getSignalsGroup()
    {
        if (signals_group.valid())
        {
            return signals_group.get();
        }

        return nullptr;
    }

    void create_pagedLODs(const settings_t &settings);

    std::vector<AnimationManager *> animation_mangers;

public slots:

    void slotUpdateSignal(QByteArray data);

private:

    QMap<QString, TrafficLight *> traffic_lights;

    osg::ref_ptr<osg::Group> signals_group = new osg::Group;

    QMap<QString, QString> signal_nodes_paths;

    std::string models_dir;

    std::string animations_dir;

    void printSignalInfo(TrafficLight *tl);


};

#endif
