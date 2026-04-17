#include "signals-data-types.h"

#include "enter-signal.h"
#include "exit-signal.h"
#include "line-signal.h"
#include "route-signal.h"
#include "shunting-signal.h"

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>

#include <cstdint>

static void serialize_signals(
    QDataStream& stream,
    std::vector<Signal*>& signals_
)
{
    stream << static_cast<std::uint32_t>(signals_.size());
    for (Signal* signal : signals_)
    {
        signal->calcPosition();
        stream << signal->serialize();
    }
}

template <typename SignalType>
static void deserialize_signals(
    QDataStream& stream,
    std::vector<Signal*>& signals_
)
{
    std::uint32_t size = 0;
    stream >> size;
    signals_.clear();

    for (std::uint32_t i = 0; i < size; ++i)
    {
        QByteArray tmp_data;
        stream >> tmp_data;

        SignalType* signal = new SignalType;
        signal->deserialize(tmp_data);

        signals_.push_back(signal);
    }
}

QByteArray signals_data_t::serialize()
{
    QByteArray tmp_data;
    QBuffer buff(&tmp_data);
    buff.open(QIODevice::WriteOnly);
    QDataStream stream(&buff);

    serialize_signals(stream, line_signals);
    serialize_signals(stream, enter_signals);
    serialize_signals(stream, route_signals);
    serialize_signals(stream, exit_signals);
    serialize_signals(stream, shunt_signals);

    return buff.data();
}

void signals_data_t::deserialize(QByteArray& data)
{
    QBuffer buff(&data);
    buff.open(QIODevice::ReadOnly);
    QDataStream stream(&buff);

    deserialize_signals<LineSignal>(stream, line_signals);
    deserialize_signals<EnterSignal>(stream, enter_signals);
    deserialize_signals<RouteSignal>(stream, route_signals);
    deserialize_signals<ExitSignal>(stream, exit_signals);
    deserialize_signals<ShuntingSignal>(stream, shunt_signals);
}
