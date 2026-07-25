#ifndef SIMULATOR_UPDATE_STRUCT_H
#define SIMULATOR_UPDATE_STRUCT_H

#include <datetime.h>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QString>

#include <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_update_players_t final
{
    std::vector<int> clients_id;
    std::vector<int> current_vehicles;
    std::vector<int> controlled_vehicles;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << static_cast<std::uint32_t>(clients_id.size());
        for (auto id : clients_id)
        {
            stream << id;
        }

        stream << static_cast<std::uint32_t>(current_vehicles.size());
        for (auto veh : current_vehicles)
        {
            stream << veh;
        }

        stream << static_cast<std::uint32_t>(controlled_vehicles.size());
        for (auto veh : controlled_vehicles)
        {
            stream << veh;
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        std::uint32_t num;

        stream >> num;
        clients_id.clear();
        clients_id.resize(num);
        for (auto& client_id : clients_id)
        {
            stream >> client_id;
        }

        stream >> num;
        current_vehicles.clear();
        current_vehicles.resize(num);
        for (auto& current_vehicle : current_vehicles)
        {
            stream >> current_vehicle;
        }

        stream >> num;
        controlled_vehicles.clear();
        controlled_vehicles.resize(num);
        for (auto& controlled_vehicle : controlled_vehicles)
        {
            stream >> controlled_vehicle;
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_pos_update_t final
{
    double  position_x = 0.0;
    double  position_y = 0.0;
    double  position_z = 0.0;
    double  orth_x = 0.0;
    double  orth_y = 0.0;
    double  orth_z = 0.0;
    double  up_x = 0.0;
    double  up_y = 0.0;
    double  up_z = 1.0;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        // Сериализация с плотной упаковкой данных
        serialize_position(stream, position_x);
        serialize_position(stream, position_y);
        serialize_position(stream, position_z);

        MaxComponent max_orth = select_max_component(orth_x, orth_y, orth_z);
        MaxComponent max_up = select_max_component(up_x, up_y, up_z);
        uint8_t max_info = max_orth | (max_up << 4);
        stream << max_info;

        serialize_vector(stream, orth_x, orth_y, orth_z, max_orth);
        serialize_vector(stream, up_x, up_y, up_z, max_up);
/*
        stream << position_x;
        stream << position_y;
        stream << position_z;
        float tmp;
        tmp = static_cast<float>(orth_x);
        stream << tmp;
        tmp = static_cast<float>(orth_y);
        stream << tmp;
        tmp = static_cast<float>(orth_z);
        stream << tmp;
        tmp = static_cast<float>(up_x);
        stream << tmp;
        tmp = static_cast<float>(up_y);
        stream << tmp;
        tmp = static_cast<float>(up_z);
        stream << tmp;

<<<<<<< HEAD
        return buff.data();
*/
        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        deserialize_position(stream, position_x);
        deserialize_position(stream, position_y);
        deserialize_position(stream, position_z);

        uint8_t max_info;
        stream >> max_info;
        MaxComponent max_orth = static_cast<MaxComponent>(max_info & 0xF);
        MaxComponent max_up = static_cast<MaxComponent>(max_info >> 4);

        deserialize_vector(stream, orth_x, orth_y, orth_z, max_orth);
        deserialize_vector(stream, up_x, up_y, up_z, max_up);
/*
        stream >> position_x;
        stream >> position_y;
        stream >> position_z;
        float tmp;
        stream >> tmp;
        orth_x = static_cast<double>(tmp);
        stream >> tmp;
        orth_y = static_cast<double>(tmp);
        stream >> tmp;
        orth_z = static_cast<double>(tmp);
        stream >> tmp;
        up_x = static_cast<double>(tmp);
        stream >> tmp;
        up_y = static_cast<double>(tmp);
        stream >> tmp;
        up_z = static_cast<double>(tmp);
*/
    }

private:

    // Константы для сжатой сериализации положения
    static constexpr uint8_t position_bytes = 5; // 5 байт вместо 8-байтного double
    static constexpr uint64_t position_shift = 1ll << (position_bytes * 8 - 1); // смещаем в положительные значения, не заморачиваемся со знаком минус
    static constexpr double position_scale = 5000.0; // Умножением на 5000 получаем точность 0.2 миллиметра
    static constexpr double position_unscale = 1.0 / position_scale;

    void serialize_position(QDataStream& stream, const double& coord) const
    {
        uint64_t coord_scaled = coord * position_scale + position_shift;
        for(int i = 0; i < position_bytes; ++i)
        {
            uint8_t byte = (coord_scaled >> (i * 8)) & 0xFF;
            stream << byte;
        }
    }
    void deserialize_position(QDataStream& stream, double& coord) const
    {
        uint64_t coord_scaled = 0;
        for(int i = 0; i < position_bytes; ++i)
        {
            uint8_t byte;
            stream >> byte;
            coord_scaled |= (static_cast<uint64_t>(byte) << (i * 8));
        }
        coord = (static_cast<double>(coord_scaled) - position_shift) * position_unscale;
    }

    // Константы для сжатой сериализации единичных векторов
    enum MaxComponent : uint8_t
    {
        MAX_X_POSITIVE = 1,
        MAX_X_NEGATIVE,
        MAX_Y_POSITIVE,
        MAX_Y_NEGATIVE,
        MAX_Z_POSITIVE,
        MAX_Z_NEGATIVE
    };
    static constexpr uint8_t vector_bytes = 2; // Храним два компонента вектора в 2*2=4 байтах вместо трёх 8-байтных double
    static constexpr uint32_t vector_shift = 1 << (vector_bytes * 8 - 1); // смещаем в положительные значения, не заморачиваемся со знаком минус
    static constexpr double vector_scale = static_cast<double>(vector_shift) / 0.7071067811865475244;
    static constexpr double vector_unscale = 1.0 / vector_scale;

    MaxComponent select_max_component(const double& vx, const double& vy, const double& vz) const
    {
        // Выбираем наибольший из компонент вектора, который не будем отправлять,
        // а восстановим по единичной длине. Выбираем наибольший, чтобы была
        // наименьшая погрешность, а значение двух других компонент не может
        // превышать 1.0 / sqrt(2.0), используем это для ещё большей точности
        MaxComponent cmax_info;
        double cmax_value;
        if (vx < 0.0)
        {
            cmax_info = MAX_X_NEGATIVE;
            cmax_value = -vx;
        }
        else
        {
            cmax_info = MAX_X_POSITIVE;
            cmax_value = vx;
        }

        if (vy < 0.0)
        {
            if (cmax_value < -vy)
            {
                cmax_info = MAX_Y_NEGATIVE;
                cmax_value = -vy;
            }
        }
        else
        {
            if (cmax_value < vy)
            {
                cmax_info = MAX_Y_POSITIVE;
                cmax_value = vy;
            }
        }

        if (vz < 0.0)
        {
            if (cmax_value < -vz)
            {
                cmax_info = MAX_Z_NEGATIVE;
            }
        }
        else
        {
            if (cmax_value < vz)
            {
                cmax_info = MAX_Z_POSITIVE;
            }
        }
        return cmax_info;
    }

    void serialize_vector(QDataStream& stream, const double& vx, const double& vy, const double& vz, const MaxComponent& cmax_info) const
    {
        auto write = [](QDataStream& stream, const double& c1, const double& c2)
        {
            uint16_t component_scaled;
            component_scaled = c1 * vector_scale + vector_shift;
            stream << component_scaled;
            component_scaled = c2 * vector_scale + vector_shift;
            stream << component_scaled;
        };
        switch (cmax_info)
        {
        case MAX_X_POSITIVE:
        case MAX_X_NEGATIVE:
        {
            write(stream, vy, vz);
            return;
        }
        case MAX_Y_POSITIVE:
        case MAX_Y_NEGATIVE:
        {
            write(stream, vx, vz);
            return;
        }
        case MAX_Z_POSITIVE:
        case MAX_Z_NEGATIVE:
        {
            write(stream, vx, vy);
            return;
        }
        }
    }

    void deserialize_vector(QDataStream& stream, double& vx, double& vy, double& vz, const MaxComponent& cmax_info) const
    {
        auto read = [](QDataStream& stream, double& cmax, double& c1, double& c2)
        {
            uint16_t component_scaled;
            stream >> component_scaled;
            c1 = (static_cast<double>(component_scaled) - vector_shift) * vector_unscale;
            stream >> component_scaled;
            c2 = (static_cast<double>(component_scaled) - vector_shift) * vector_unscale;

            // Восстанавливаем третий компонент вектора по единичной длине
            cmax = std::sqrt(std::max(0.0, 1.0 - c1 * c1 - c2 * c2));
        };

        switch (cmax_info)
        {
        case MAX_X_POSITIVE:
        {
            read(stream, vx, vy, vz);
            return;
        }
        case MAX_X_NEGATIVE:
        {
            read(stream, vx, vy, vz);
            vx = -vx;
            return;
        }
        case MAX_Y_POSITIVE:
        {
            read(stream, vy, vx, vz);
            return;
        }
        case MAX_Y_NEGATIVE:
        {
            read(stream, vy, vx, vz);
            vy = -vy;
            return;
        }
        case MAX_Z_POSITIVE:
        {
            read(stream, vz, vx, vy);
            return;
        }
        case MAX_Z_NEGATIVE:
        {
            read(stream, vz, vx, vy);
            vz = -vz;
            return;
        }
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_update_pos_t final
{
    int speed_factor = 1;
    simulator_time_t sim_time;
    std::vector<simulator_vehicle_pos_update_t> vehicles;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << speed_factor;
        stream << sim_time.serialize();

        stream << static_cast<std::uint32_t>(vehicles.size());
        for (const auto& vehicle_pos : vehicles)
        {
            stream << vehicle_pos.serialize();
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> speed_factor;

        QByteArray sim_time_data;
        stream >> sim_time_data;
        sim_time.deserialize(sim_time_data);

        std::uint32_t num;

        stream >> num;
        vehicles.clear();
        vehicles.resize(num);

        for (auto& vehicle : vehicles)
        {
            QByteArray vehicle_data;
            stream >> vehicle_data;

            vehicle.deserialize(vehicle_data);
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_update_t final
{
    int orientation = 1;
    int train_id = 0;
    int prev_vehicle = -1;
    int next_vehicle = -1;
    std::vector<float> analogSignal;
    /// Сериализованные данные графика движения
    QByteArray timetableData;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << orientation;
        stream << train_id;
        stream << prev_vehicle;
        stream << next_vehicle;

        stream << static_cast<std::uint32_t>(analogSignal.size());

        for (auto signal : analogSignal)
        {
            stream << signal;
        }

        quint32 tt_data_size = timetableData.size();
        stream << tt_data_size;

        if (tt_data_size != 0)
        {
            stream << timetableData;
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> orientation;
        stream >> train_id;
        stream >> prev_vehicle;
        stream >> next_vehicle;

        std::uint32_t num;
        stream >> num;
        analogSignal.clear();
        analogSignal.resize(num);

        for (auto& signal : analogSignal)
        {
            stream >> signal;
        }

        quint32 tt_data_size = 0;
        stream >> tt_data_size;

        if (tt_data_size != 0)
        {
            stream >> timetableData;
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicles_update_t final
{
    std::vector<simulator_vehicle_update_t> vehicles;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << static_cast<std::uint32_t>(vehicles.size());

        for (const auto& vehicle : vehicles)
        {
            stream << vehicle.serialize();
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        std::uint32_t num;
        stream >> num;

        vehicles.clear();
        vehicles.resize(num);

        for (auto& vehicle : vehicles)
        {
            QByteArray vehicle_data;
            stream >> vehicle_data;

            vehicle.deserialize(vehicle_data);
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_train_update_t final
{
    int  first_vehicle_id = 0;
    int  last_vehicle_id = 0;
    QString train_name = "";

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << first_vehicle_id;
        stream << last_vehicle_id;
        stream << train_name;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> first_vehicle_id;
        stream >> last_vehicle_id;
        stream >> train_name;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_trains_update_t final
{
    std::vector<simulator_train_update_t> trains;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << static_cast<std::uint32_t>(trains.size());

        for (const auto& train : trains)
        {
            stream << train.serialize();
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        std::uint32_t num;
        stream >> num;

        trains.clear();
        trains.resize(num);

        for (auto& train : trains)
        {
            QByteArray train_data;
            stream >> train_data;

            train.deserialize(train_data);
        }
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_vehicle_controlled_update_t final
{
    int current_vehicle = 0;
    QString currentDebugMsg;
    int controlled_vehicle = 0;
    QString controlledDebugMsg;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << current_vehicle;
        stream << currentDebugMsg;
        stream << controlled_vehicle;
        stream << controlledDebugMsg;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> current_vehicle;
        stream >> currentDebugMsg;
        stream >> controlled_vehicle;
        stream >> controlledDebugMsg;
    }
};

#endif // SIMULATOR_UPDATE_STRUCT_H
