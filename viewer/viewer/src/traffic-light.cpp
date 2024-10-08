#include	<traffic-light.h>
#include    <QBuffer>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLight::TrafficLight(QObject *parent) : QObject(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrafficLight::~TrafficLight()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrafficLight::deserialize(QByteArray &data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    stream >> conn_name;
    stream >> signal_dir;
    stream >> is_busy;
    stream >> letter;
    stream >> signal_model;

    for (size_t i = 0; i < lens_state.size(); ++i)
    {
        stream >> lens_state[i];
    }
}

