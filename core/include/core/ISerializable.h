#ifndef CORE_ISERIALIZABLE_H
#define CORE_ISERIALIZABLE_H

#include <QByteArray>

class QDataStream;

class ISerializable
{
public:
    virtual ~ISerializable();
    virtual void serialize(QDataStream& stream) const = 0;
    virtual void deserialize(QDataStream& stream) = 0;

    QByteArray to_byte_array() const;
    void from_byte_array(QByteArray& data);
};

#endif // CORE_ISERIALIZABLE_H
