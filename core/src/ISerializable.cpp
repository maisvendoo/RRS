#include "core/ISerializable.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

ISerializable::~ISerializable() = default;

QByteArray ISerializable::to_byte_array() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    serialize(stream);
    return data;
}

void ISerializable::from_byte_array(QByteArray& data)
{
    QDataStream stream(&data, QIODevice::ReadOnly);
    deserialize(stream);
}
