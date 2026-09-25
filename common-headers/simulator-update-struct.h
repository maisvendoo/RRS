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

    /// ╨а╨╡╨░╨║╤Ж╨╕╤П ╨║╨░╨╝╨╡╤А╤Л ╨╛╤В ╤Д╨╕╨╖╨╕╨║╨╕.
    /// ╨Э╨╛╨▓╤Л╨╡ ╨┐╨╛╨╗╤П ╨┤╨╛╨▒╨░╨▓╨╗╨╡╨╜╤Л ╨Т ╨Ъ╨Ю╨Э╨Х╨ж ╨┤╨╗╤П ╨╛╨▒╤А╨░╤В╨╜╨╛╨╣ ╤Б╨╛╨▓╨╝╨╡╤Б╤В╨╕╨╝╨╛╤Б╤В╨╕:
    /// ╤Б╤В╨░╤А╤Л╨╣ ╨║╨╗╨╕╨╡╨╜╤В ╨┐╤А╨╛╤Б╤В╨╛ ╨╜╨╡ ╤З╨╕╤В╨░╨╡╤В ╤Е╨▓╨╛╤Б╤В ╨▓╨╗╨╛╨╢╨╡╨╜╨╜╨╛╨│╨╛ ╨▒╨╗╨╛╨║╨░
    float   cam_offset_x = 0.0f;    ///< ╨б╨╝╨╡╤Й╨╡╨╜╨╕╨╡ ╨│╨╛╨╗╨╛╨▓╤Л: X - ╨┐╤А╨╛╨┤╨╛╨╗╤М╨╜╨╛╨╡, ╨╝
    float   cam_offset_y = 0.0f;    ///< Y - ╨┐╨╛╨┐╨╡╤А╨╡╤З╨╜╨╛╨╡, ╨╝
    float   cam_offset_z = 0.0f;    ///< Z - ╨▓╨╡╤А╤В╨╕╨║╨░╨╗╤М╨╜╨╛╨╡, ╨╝
    float   cam_tilt_roll = 0.0f;   ///< ╨Э╨░╨║╨╗╨╛╨╜ ╨║╤А╨╡╨╜, ╤А╨░╨┤
    float   cam_tilt_pitch = 0.0f;  ///< ╨Э╨░╨║╨╗╨╛╨╜ ╤В╨░╨╜╨│╨░╨╢, ╤А╨░╨┤

    /// ╨Ф╤Л╨╝╨╜╨╛╤Б╤В╤М ╨Я╨Х ╨┤╨╗╤П ╤А╨╡╨╜╨┤╨╡╤А╨░ ╤З╨░╤Б╤В╨╕╤Ж: ╤Г╤А╨╛╨▓╨╡╨╜╤М 0..4 -
    /// Smoke ╨╕╨╖ DieselEngineSystem/SteamEngineSystem; ╨║╨╛╨┤ ╤Ж╨▓╨╡╤В╨░ ╨┤╤Л╨╝╨░:
    /// 0 - ╨╜╨╡╤В, 1 - ╤З╤С╤А╨╜╤Л╨╣, 2 - ╤Б╨╕╨╜╨╕╨╣, 3 - ╨▒╨╡╨╗╤Л╨╣, 4 - ╤Б╨╡╤А╤Л╨╣.
    /// ╨Ч╨░╨┐╨╛╨╗╨╜╤П╨╡╤В╤Б╤П ╤Б╨╡╤А╨▓╨╡╤А╨╛╨╝ ╨╕╨╖ Vehicle::getSteam()/getDiesel()
    /// (╨┐╨░╤А╨╛╨▓╨╛╨╖ ╨╕╨╝╨╡╨╡╤В ╨┐╤А╨╕╨╛╤А╨╕╤В╨╡╤В). ╨Ф╨╛╨▒╨░╨▓╨╗╨╡╨╜╨╛ ╨Т ╨Ъ╨Ю╨Э╨Х╨ж ╨┤╨╗╤П ╨╛╨▒╤А╨░╤В╨╜╨╛╨╣
    /// ╤Б╨╛╨▓╨╝╨╡╤Б╤В╨╕╨╝╨╛╤Б╤В╨╕ ╨┐╤А╨╛╤В╨╛╨║╨╛╨╗╨░
    quint8  smoke_level = 0;
    quint8  smoke_color = 0;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        // ╨б╨╡╤А╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╤П ╤Б ╨┐╨╗╨╛╤В╨╜╨╛╨╣ ╤Г╨┐╨░╨║╨╛╨▓╨║╨╛╨╣ ╨┤╨░╨╜╨╜╤Л╤Е
        serialize_position(stream, position_x);
        serialize_position(stream, position_y);
        serialize_position(stream, position_z);

        MaxComponent max_orth = select_max_component(orth_x, orth_y, orth_z);
        MaxComponent max_up = select_max_component(up_x, up_y, up_z);
        uint8_t max_info = max_orth | (max_up << 4);
        stream << max_info;

        serialize_vector(stream, orth_x, orth_y, orth_z, max_orth);
        serialize_vector(stream, up_x, up_y, up_z, max_up);

        // ╨а╨╡╨░╨║╤Ж╨╕╤П ╨║╨░╨╝╨╡╤А╤Л ╨╛╤В ╤Д╨╕╨╖╨╕╨║╨╕ (╨┤╨╛╨▒╨░╨▓╨╗╨╡╨╜╨╛ ╨▓ ╨║╨╛╨╜╨╡╤Ж, ╤Б╨╝. ╨║╨╛╨╝╨╝╨╡╨╜╤В╨░╤А╨╕╨╣ ╨▓╤Л╤И╨╡)
        stream << cam_offset_x;
        stream << cam_offset_y;
        stream << cam_offset_z;
        stream << cam_tilt_roll;
        stream << cam_tilt_pitch;

        // ╨Ф╤Л╨╝╨╜╨╛╤Б╤В╤М ╨Я╨н ╨┤╨╗╤П ╤А╨╡╨╜╨┤╨╡╤А╨░ ╤З╨░╤Б╤В╨╕╤Ж (╨┤╨╛╨▒╨░╨▓╨╗╨╡╨╜╨╛ ╨▓ ╨║╨╛╨╜╨╡╤Ж)
        stream << smoke_level;
        stream << smoke_color;
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
*/
        return data;
    }

    void deserialize(QByteArray& data)
    {
        // ╨Ч╨╜╨░╤З╨╡╨╜╨╕╤П ╨┐╨╛ ╤Г╨╝╨╛╨╗╤З╨░╨╜╨╕╤О: ╤Б╤В╨░╤А╤Л╨╣ ╤Б╨╡╤А╨▓╨╡╤А ╨╜╨╡ ╨┐╤А╨╕╤И╨╗╤С╤В ╤Н╤В╨╕ ╨┐╨╛╨╗╤П
        cam_offset_x = 0.0f;
        cam_offset_y = 0.0f;
        cam_offset_z = 0.0f;
        cam_tilt_roll = 0.0f;
        cam_tilt_pitch = 0.0f;
        smoke_level = 0;
        smoke_color = 0;

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

        // ╨а╨╡╨░╨║╤Ж╨╕╤П ╨║╨░╨╝╨╡╤А╤Л ╨╛╤В ╤Д╨╕╨╖╨╕╨║╨╕: ╤Е╨▓╨╛╤Б╤В ╨▒╨╗╨╛╨║╨░, ╨╛╤В╤Б╤Г╤В╤Б╤В╨▓╤Г╨╡╤В ╤Г ╤Б╤В╨░╤А╨╛╨│╨╛ ╤Б╨╡╤А╨▓╨╡╤А╨░
        if (!stream.atEnd())
        {
            stream >> cam_offset_x;
            stream >> cam_offset_y;
            stream >> cam_offset_z;
            stream >> cam_tilt_roll;
            stream >> cam_tilt_pitch;
        }

        // ╨Ф╤Л╨╝╨╜╨╛╤Б╤В╤М ╨Я╨н: ╤Е╨▓╨╛╤Б╤В ╨▒╨╗╨╛╨║╨░ ╨╜╨╛╨▓╨╛╨│╨╛ ╨┐╤А╨╛╤В╨╛╨║╨╛╨╗╨░
        if (!stream.atEnd())
        {
            stream >> smoke_level;
            stream >> smoke_color;
        }
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

    // ╨Ъ╨╛╨╜╤Б╤В╨░╨╜╤В╤Л ╨┤╨╗╤П ╤Б╨╢╨░╤В╨╛╨╣ ╤Б╨╡╤А╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╨╕ ╨┐╨╛╨╗╨╛╨╢╨╡╨╜╨╕╤П
    static constexpr uint8_t position_bytes = 5; // 5 ╨▒╨░╨╣╤В ╨▓╨╝╨╡╤Б╤В╨╛ 8-╨▒╨░╨╣╤В╨╜╨╛╨│╨╛ double
    static constexpr uint64_t position_shift = 1ll << (position_bytes * 8 - 1); // ╤Б╨╝╨╡╤Й╨░╨╡╨╝ ╨▓ ╨┐╨╛╨╗╨╛╨╢╨╕╤В╨╡╨╗╤М╨╜╤Л╨╡ ╨╖╨╜╨░╤З╨╡╨╜╨╕╤П, ╨╜╨╡ ╨╖╨░╨╝╨╛╤А╨░╤З╨╕╨▓╨░╨╡╨╝╤Б╤П ╤Б╨╛ ╨╖╨╜╨░╨║╨╛╨╝ ╨╝╨╕╨╜╤Г╤Б
    static constexpr double position_scale = 5000.0; // ╨г╨╝╨╜╨╛╨╢╨╡╨╜╨╕╨╡╨╝ ╨╜╨░ 5000 ╨┐╨╛╨╗╤Г╤З╨░╨╡╨╝ ╤В╨╛╤З╨╜╨╛╤Б╤В╤М 0.2 ╨╝╨╕╨╗╨╗╨╕╨╝╨╡╤В╤А╨░
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

    // ╨Ъ╨╛╨╜╤Б╤В╨░╨╜╤В╤Л ╨┤╨╗╤П ╤Б╨╢╨░╤В╨╛╨╣ ╤Б╨╡╤А╨╕╨░╨╗╨╕╨╖╨░╤Ж╨╕╨╕ ╨╡╨┤╨╕╨╜╨╕╤З╨╜╤Л╤Е ╨▓╨╡╨║╤В╨╛╤А╨╛╨▓
    enum MaxComponent : uint8_t
    {
        MAX_X_POSITIVE = 1,
        MAX_X_NEGATIVE,
        MAX_Y_POSITIVE,
        MAX_Y_NEGATIVE,
        MAX_Z_POSITIVE,
        MAX_Z_NEGATIVE
    };
    static constexpr uint8_t vector_bytes = 2; // ╨е╤А╨░╨╜╨╕╨╝ ╨┤╨▓╨░ ╨║╨╛╨╝╨┐╨╛╨╜╨╡╨╜╤В╨░ ╨▓╨╡╨║╤В╨╛╤А╨░ ╨▓ 2*2=4 ╨▒╨░╨╣╤В╨░╤Е ╨▓╨╝╨╡╤Б╤В╨╛ ╤В╤А╤С╤Е 8-╨▒╨░╨╣╤В╨╜╤Л╤Е double
    static constexpr uint32_t vector_shift = 1 << (vector_bytes * 8 - 1); // ╤Б╨╝╨╡╤Й╨░╨╡╨╝ ╨▓ ╨┐╨╛╨╗╨╛╨╢╨╕╤В╨╡╨╗╤М╨╜╤Л╨╡ ╨╖╨╜╨░╤З╨╡╨╜╨╕╤П, ╨╜╨╡ ╨╖╨░╨╝╨╛╤А╨░╤З╨╕╨▓╨░╨╡╨╝╤Б╤П ╤Б╨╛ ╨╖╨╜╨░╨║╨╛╨╝ ╨╝╨╕╨╜╤Г╤Б
    static constexpr double vector_scale = static_cast<double>(vector_shift) / 0.7071067811865475244;
    static constexpr double vector_unscale = 1.0 / vector_scale;

    MaxComponent select_max_component(const double& vx, const double& vy, const double& vz) const
    {
        // ╨Т╤Л╨▒╨╕╤А╨░╨╡╨╝ ╨╜╨░╨╕╨▒╨╛╨╗╤М╤И╨╕╨╣ ╨╕╨╖ ╨║╨╛╨╝╨┐╨╛╨╜╨╡╨╜╤В ╨▓╨╡╨║╤В╨╛╤А╨░, ╨║╨╛╤В╨╛╤А╤Л╨╣ ╨╜╨╡ ╨▒╤Г╨┤╨╡╨╝ ╨╛╤В╨┐╤А╨░╨▓╨╗╤П╤В╤М,
        // ╨░ ╨▓╨╛╤Б╤Б╤В╨░╨╜╨╛╨▓╨╕╨╝ ╨┐╨╛ ╨╡╨┤╨╕╨╜╨╕╤З╨╜╨╛╨╣ ╨┤╨╗╨╕╨╜╨╡. ╨Т╤Л╨▒╨╕╤А╨░╨╡╨╝ ╨╜╨░╨╕╨▒╨╛╨╗╤М╤И╨╕╨╣, ╤З╤В╨╛╨▒╤Л ╨▒╤Л╨╗╨░
        // ╨╜╨░╨╕╨╝╨╡╨╜╤М╤И╨░╤П ╨┐╨╛╨│╤А╨╡╤И╨╜╨╛╤Б╤В╤М, ╨░ ╨╖╨╜╨░╤З╨╡╨╜╨╕╨╡ ╨┤╨▓╤Г╤Е ╨┤╤А╤Г╨│╨╕╤Е ╨║╨╛╨╝╨┐╨╛╨╜╨╡╨╜╤В ╨╜╨╡ ╨╝╨╛╨╢╨╡╤В
        // ╨┐╤А╨╡╨▓╤Л╤И╨░╤В╤М 1.0 / sqrt(2.0), ╨╕╤Б╨┐╨╛╨╗╤М╨╖╤Г╨╡╨╝ ╤Н╤В╨╛ ╨┤╨╗╤П ╨╡╤Й╤С ╨▒╨╛╨╗╤М╤И╨╡╨╣ ╤В╨╛╤З╨╜╨╛╤Б╤В╨╕
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

            // ╨Т╨╛╤Б╤Б╤В╨░╨╜╨░╨▓╨╗╨╕╨▓╨░╨╡╨╝ ╤В╤А╨╡╤В╨╕╨╣ ╨║╨╛╨╝╨┐╨╛╨╜╨╡╨╜╤В ╨▓╨╡╨║╤В╨╛╤А╨░ ╨┐╨╛ ╨╡╨┤╨╕╨╜╨╕╤З╨╜╨╛╨╣ ╨┤╨╗╨╕╨╜╨╡
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
// ╨д╨╕╨╖╨╕╤З╨╡╤Б╨║╨╛╨╡ ╨╖╨▓╤Г╨║╨╛╨▓╨╛╨╡ ╤Б╨╛╨▒╤Л╤В╨╕╨╡ ╨┤╨╗╤П ╨░╤Г╨┤╨╕╨╛╤Б╨╕╤Б╤В╨╡╨╝╤Л ╨║╨╗╨╕╨╡╨╜╤В╨░
// ( wire-╨┐╤А╨╡╨┤╤Б╤В╨░╨▓╨╗╨╡╨╜╨╕╨╡ SoundEvent ╨╕╨╖ simulator: ╤В╨╕╨┐╤Л 0..9 ╤Б╨╛╨▓╨┐╨░╨┤╨░╤О╤В
//   ╤Б SoundEventType ╨▓ vehicle-sound-events.h )
//------------------------------------------------------------------------------
struct simulator_sound_event_t final
{
    quint8  type = 0;           ///< ╨в╨╕╨┐ ╤Б╨╛╨▒╤Л╤В╨╕╤П (SoundEventType)
    float   x = 0.0f;           ///< ╨Ь╨╕╤А╨╛╨▓╨░╤П ╨┐╨╛╨╖╨╕╤Ж╨╕╤П, ╨╝
    float   y = 0.0f;
    float   z = 0.0f;
    float   intensity = 0.0f;   ///< ╨б╨╕╨╗╨░/╨│╤А╨╛╨╝╨║╨╛╤Б╤В╤М 0..1
    float   rate_hz = 0.0f;     ///< ╨з╨░╤Б╤В╨╛╤В╨░ ╨┐╨╛╨▓╤В╨╛╤А╨░, ╨У╤Ж
    quint32 vehicle_idx = 0;    ///< ╨Ш╨╜╨┤╨╡╨║╤Б ╨Я╨Х-╨╕╤Б╤В╨╛╤З╨╜╨╕╨║╨░

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << type;
        stream << x;
        stream << y;
        stream << z;
        stream << intensity;
        stream << rate_hz;
        stream << vehicle_idx;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> type;
        stream >> x;
        stream >> y;
        stream >> z;
        stream >> intensity;
        stream >> rate_hz;
        stream >> vehicle_idx;
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

    /// ╨Я╨╛╨│╨╛╨┤╨░. ╨Э╨╛╨▓╤Л╨╡ ╨┐╨╛╨╗╤П ╨┤╨╛╨▒╨░╨▓╨╗╨╡╨╜╤Л ╨Т ╨Ъ╨Ю╨Э╨Х╨ж
    /// ╤Б╤В╤А╤Г╨║╤В╤Г╤А╤Л ╨┤╨╗╤П ╨╛╨▒╤А╨░╤В╨╜╨╛╨╣ ╤Б╨╛╨▓╨╝╨╡╤Б╤В╨╕╨╝╨╛╤Б╤В╨╕ ╨┐╤А╨╛╤В╨╛╨║╨╛╨╗╨░
    float visibility_m = 10000.0f;  ///< ╨Ф╨░╨╗╤М╨╜╨╛╤Б╤В╤М ╨▓╨╕╨┤╨╕╨╝╨╛╤Б╤В╨╕, ╨╝
    float fog_density = 0.0f;       ///< ╨Я╨╗╨╛╤В╨╜╨╛╤Б╤В╤М ╤В╤Г╨╝╨░╨╜╨░, 1/╨╝

    /// ╨Я╨╛╨│╨╛╨┤╨░ ╨┤╨╗╤П ╤Н╤Д╤Д╨╡╨║╤В╨╛╨▓ ╤А╨╡╨╜╨┤╨╡╤А╨░: ╤В╨╕╨┐ ╨┐╨╛╨│╨╛╨┤╤Л 0..15 -
    /// ╨╖╨╜╨░╤З╨╡╨╜╨╕╤П ╤Б╨╛╨│╨╗╨░╤Б╨╛╨▓╨░╨╜╤Л ╤Б weather::Type (simulator/weather/include/
    /// weather-system.h); ╨╕╨╜╤В╨╡╨╜╤Б╨╕╨▓╨╜╨╛╤Б╤В╤М 0..1 ╨╕ ╨▓╨╡╤В╨╡╤А (╤Б╨║╨╛╤А╨╛╤Б╤В╤М, ╨╝/╤Б;
    /// ╨╜╨░╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╨╡ - ╨░╨╖╨╕╨╝╤Г╤В, ╤А╨░╨┤). ╨Ф╨╛╨▒╨░╨▓╨╗╨╡╨╜╨╛ ╨Т ╨Ъ╨Ю╨Э╨Х╨ж ╨┤╨╗╤П ╨╛╨▒╤А╨░╤В╨╜╨╛╨╣
    /// ╤Б╨╛╨▓╨╝╨╡╤Б╤В╨╕╨╝╨╛╤Б╤В╨╕ ╨┐╤А╨╛╤В╨╛╨║╨╛╨╗╨░
    quint8  weather_type = 0;
    float   weather_intensity = 0.0f;
    float   wind_speed = 0.0f;
    float   wind_direction = 0.0f;

    /// ╨д╨╕╨╖╨╕╤З╨╡╤Б╨║╨╕╨╡ ╨╖╨▓╤Г╨║╨╛╨▓╤Л╨╡ ╤Б╨╛╨▒╤Л╤В╨╕╤П ╨┐╨╛╤Б╨╗╨╡╨┤╨╜╨╡╨│╨╛ ╤И╨░╨│╨░
    std::vector<simulator_sound_event_t> sound_events;

    /// ╨б╨╗╤Г╨╢╨╡╨▒╨╜╨╛╨╡ ╨┐╤А╨╡╨┤╤Г╨┐╤А╨╡╨╢╨┤╨╡╨╜╨╕╨╡ ╨╕╨│╤А╨╛╨║╤Г (╨║╨░╤Б╤Б╨╡╤В╨░ ╤А╨╡╨│╨╕╤Б╤В╤А╨░╤Ж╨╕╨╕: ╨╜╨░╤З╨░╨╗╨╛/ / ╨╛╨║╨╛╨╜╤З╨░╨╜╨╕╨╡ ╨╖╨░╨┐╨╕╤Б╨╕). notice_id ╤А╨░╤Б╤В╤С╤В ╨┐╤А╨╕ ╨║╨░╨╢╨┤╨╛╨╝
    /// ╨╜╨╛╨▓╨╛╨╝ ╤Б╨╛╨╛╨▒╤Й╨╡╨╜╨╕╨╕; 0 - ╤Б╨╛╨╛╨▒╤Й╨╡╨╜╨╕╨╣ ╨╡╤Й╤С ╨╜╨╡ ╨▒╤Л╨╗╨╛
    quint32 notice_id = 0;
    QString notice = "";

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

        // ╨Я╨╛╨│╨╛╨┤╨░ ╨╕ ╨╖╨▓╤Г╨║╨╛╨▓╤Л╨╡ ╤Б╨╛╨▒╤Л╤В╨╕╤П (╨┤╨╛╨▒╨░╨▓╨╗╨╡╨╜╨╛ ╨▓ ╨║╨╛╨╜╨╡╤Ж, ╤Б╨╝. ╨▓╤Л╤И╨╡)
        stream << visibility_m;
        stream << fog_density;

        stream << static_cast<std::uint32_t>(sound_events.size());
        for (const auto& event : sound_events)
        {
            stream << event.serialize();
        }

        stream << notice_id;
        stream << notice;

        // ╨Я╨╛╨│╨╛╨┤╨░ ╨┤╨╗╤П ╤Н╤Д╤Д╨╡╨║╤В╨╛╨▓ ╤А╨╡╨╜╨┤╨╡╤А╨░: ╤Б╨░╨╝╤Л╨╣ ╤Е╨▓╨╛╤Б╤В ╨┐╤А╨╛╤В╨╛╨║╨╛╨╗╨░, ╤З╨╕╤В╨░╨╡╤В╤Б╤П
        // ╤В╨╛╨╗╤М╨║╨╛ ╨╡╤Б╨╗╨╕ ╨┤╨░╨╜╨╜╤Л╨╡ ╨╡╤Й╤С ╨╛╤Б╤В╨░╨╗╨╕╤Б╤М (╤Б╤В╨░╤А╤Л╨╣ ╤Б╨╡╤А╨▓╨╡╤А ╨╕╤Е ╨╜╨╡ ╨┐╤А╨╕╤И╨╗╤С╤В)
        stream << weather_type;
        stream << weather_intensity;
        stream << wind_speed;
        stream << wind_direction;

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

        // ╨Я╨╛╨│╨╛╨┤╨░ ╨╕ ╨╖╨▓╤Г╨║╨╛╨▓╤Л╨╡ ╤Б╨╛╨▒╤Л╤В╨╕╤П: ╤Е╨▓╨╛╤Б╤В ╨┐╨░╨║╨╡╤В╨░, ╨╛╤В╤Б╤Г╤В╤Б╤В╨▓╤Г╨╡╤В ╤Г ╤Б╤В╨░╤А╨╛╨│╨╛ ╤Б╨╡╤А╨▓╨╡╤А╨░
        visibility_m = 10000.0f;
        fog_density = 0.0f;
        sound_events.clear();
        notice_id = 0;
        notice.clear();
        weather_type = 0;
        weather_intensity = 0.0f;
        wind_speed = 0.0f;
        wind_direction = 0.0f;

        if (!stream.atEnd())
        {
            stream >> visibility_m;
            stream >> fog_density;
        }

        if (!stream.atEnd())
        {
            stream >> num;
            sound_events.resize(num);

            for (auto& event : sound_events)
            {
                QByteArray event_data;
                stream >> event_data;
                event.deserialize(event_data);
            }
        }

        // ╨Я╤А╨╡╨┤╤Г╨┐╤А╨╡╨╢╨┤╨╡╨╜╨╕╨╡ (╨║╨░╤Б╤Б╨╡╤В╨░): ╤Е╨▓╨╛╤Б╤В ╨╜╨╛╨▓╨╛╨│╨╛ ╨┐╤А╨╛╤В╨╛╨║╨╛╨╗╨░
        if (!stream.atEnd())
        {
            stream >> notice_id;
            stream >> notice;
        }

        // ╨Я╨╛╨│╨╛╨┤╨░ ╨┤╨╗╤П ╤Н╤Д╤Д╨╡╨║╤В╨╛╨▓ ╤А╨╡╨╜╨┤╨╡╤А╨░: ╤Е╨▓╨╛╤Б╤В ╨╜╨╛╨▓╨╛╨│╨╛ ╨┐╤А╨╛╤В╨╛╨║╨╛╨╗╨░
        if (!stream.atEnd())
        {
            stream >> weather_type;
            stream >> weather_intensity;
            stream >> wind_speed;
            stream >> wind_direction;
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
    /// ╨б╨╡╤А╨╕╨░╨╗╨╕╨╖╨╛╨▓╨░╨╜╨╜╤Л╨╡ ╨┤╨░╨╜╨╜╤Л╨╡ ╨│╤А╨░╤Д╨╕╨║╨░ ╨┤╨▓╨╕╨╢╨╡╨╜╨╕╤П
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

//------------------------------------------------------------------------------
// ╨Ф╨╕╨░╨│╨╜╨╛╤Б╤В╨╕╨║╨░ ╨▓╨░╨│╨╛╨╜╨░ (F3/F4).
// ╨Ъ╨╛╨╝╨┐╨░╨║╤В╨╜╨╛╨╡ wire-╨┐╤А╨╡╨┤╤Б╤В╨░╨▓╨╗╨╡╨╜╨╕╨╡ Train::VehicleDiagnostics
//------------------------------------------------------------------------------
struct simulator_vehicle_diagnostics_t final
{
    int     vehicle_idx = 0;
    float   mass_t = 0.0f;             ///< ╨Ь╨░╤Б╤Б╨░, ╤В
    float   speed_kmh = 0.0f;          ///< ╨б╨║╨╛╤А╨╛╤Б╤В╤М, ╨║╨╝/╤З
    float   force_kn = 0.0f;           ///< ╨Я╤А╨╛╨┤╨╛╨╗╤М╨╜╨╛╨╡ ╤Г╤Б╨╕╨╗╨╕╨╡ ╤Б╤Ж╨╡╨┐╨╛╨║, ╨║╨Э
    float   vertical_accel = 0.0f;     ///< ╨Т╨╡╤А╤В╨╕╨║╨░╨╗╤М╨╜╨╛╨╡ ╤Г╤Б╨║╨╛╤А╨╡╨╜╨╕╨╡, ╨╝/╤Б^2
    float   lateral_accel = 0.0f;      ///< ╨Я╨╛╨┐╨╡╤А╨╡╤З╨╜╨╛╨╡ ╤Г╤Б╨║╨╛╤А╨╡╨╜╨╕╨╡, ╨╝/╤Б^2
    float   body_damage = 0.0f;        ///< ╨Я╨╛╨▓╤А╨╡╨╢╨┤╨╡╨╜╨╕╨╡ ╨║╤Г╨╖╨╛╨▓╨░ 0..1
    float   bogie_damage = 0.0f;       ///< ╨Я╨╛╨▓╤А╨╡╨╢╨┤╨╡╨╜╨╕╨╡ ╤Е╨╛╨┤╨╛╨▓╨╛╨╣ 0..1
    float   brake_efficiency = 1.0f;   ///< ╨н╤Д╤Д╨╡╨║╤В╨╕╨▓╨╜╨╛╤Б╤В╤М ╨║╨╛╨╗╨╛╨┤╨╛╨║ 1..0
    float   shoe_temperature = 20.0f;  ///< ╨в╨╡╨╝╨┐╨╡╤А╨░╤В╤Г╤А╨░ ╨║╨╛╨╗╨╛╨┤╨╛╨║, ┬░C
    float   rail_coord_m = 0.0f;       ///< ╨Я╨╕╨║╨╡╤В╨░╨╢, ╨╝
    float   inclination = 0.0f;        ///< ╨г╨║╨╗╨╛╨╜, ╨┐╤А╨╛╨╝╨╕╨╗╨╗╨╡
    quint8  derailed = 0;              ///< ╨б╤Е╨╛╨┤
    quint8  coupled_fwd = 1;           ///< ╨б╤Ж╨╡╨┐╨╗╨╡╨╜╨░ ╤Б╨┐╨╡╤А╨╡╨┤╨╕
    quint8  coupled_bwd = 1;           ///< ╨б╤Ж╨╡╨┐╨╗╨╡╨╜╨░ ╤Б╨╖╨░╨┤╨╕

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << vehicle_idx;
        stream << mass_t;
        stream << speed_kmh;
        stream << force_kn;
        stream << vertical_accel;
        stream << lateral_accel;
        stream << body_damage;
        stream << bogie_damage;
        stream << brake_efficiency;
        stream << shoe_temperature;
        stream << rail_coord_m;
        stream << inclination;
        stream << derailed;
        stream << coupled_fwd;
        stream << coupled_bwd;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> vehicle_idx;
        stream >> mass_t;
        stream >> speed_kmh;
        stream >> force_kn;
        stream >> vertical_accel;
        stream >> lateral_accel;
        stream >> body_damage;
        stream >> bogie_damage;
        stream >> brake_efficiency;
        stream >> shoe_temperature;
        stream >> rail_coord_m;
        stream >> inclination;
        stream >> derailed;
        stream >> coupled_fwd;
        stream >> coupled_bwd;
    }
};

//------------------------------------------------------------------------------
// ╨б╨▓╨╛╨┤╨║╨░ ╨┐╤А╨╛╨┤╨╛╨╗╤М╨╜╨╛╨╣ ╨┤╨╕╨╜╨░╨╝╨╕╨║╨╕ ╤Б╨╛╤Б╤В╨░╨▓╨░ (Train::LongitudinalStats)
//------------------------------------------------------------------------------
struct simulator_train_diagnostics_t final
{
    int     first_vehicle_id = 0;
    int     last_vehicle_id = 0;
    QString train_name = "";
    float   train_mass_t = 0.0f;       ///< ╨Ь╨░╤Б╤Б╨░ ╤Б╨╛╤Б╤В╨░╨▓╨░, ╤В
    float   train_length_m = 0.0f;     ///< ╨Ф╨╗╨╕╨╜╨░ ╤Б╨╛╤Б╤В╨░╨▓╨░, ╨╝
    float   max_tension_kn = 0.0f;     ///< ╨Ь╨░╨║╤Б╨╕╨╝╤Г╨╝ ╤А╨░╤Б╤В╤П╨╢╨╡╨╜╨╕╤П ╤Б╤Ж╨╡╨┐╨╛╨║, ╨║╨Э
    float   max_compression_kn = 0.0f; ///< ╨Ь╨░╨║╤Б╨╕╨╝╤Г╨╝ ╤Б╨╢╨░╤В╨╕╤П ╤Б╤Ж╨╡╨┐╨╛╨║, ╨║╨Э
    float   max_abs_force_kn = 0.0f;   ///< ╨Ь╨░╨║╤Б╨╕╨╝╤Г╨╝ |╤Г╤Б╨╕╨╗╨╕╤П|, ╨║╨Э
    qint32  overloaded_joints = 0;     ///< ╨Я╨╡╤А╨╡╨│╤А╤Г╨╢╨╡╨╜╨╜╤Л╨╡ ╤Б╤Ж╨╡╨┐╨║╨╕
    qint32  broken_joints = 0;         ///< ╨а╨░╨╖╤А╤Г╤И╨╡╨╜╨╜╤Л╨╡ ╤Б╤Ж╨╡╨┐╨║╨╕

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << first_vehicle_id;
        stream << last_vehicle_id;
        stream << train_name;
        stream << train_mass_t;
        stream << train_length_m;
        stream << max_tension_kn;
        stream << max_compression_kn;
        stream << max_abs_force_kn;
        stream << overloaded_joints;
        stream << broken_joints;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> first_vehicle_id;
        stream >> last_vehicle_id;
        stream >> train_name;
        stream >> train_mass_t;
        stream >> train_length_m;
        stream >> max_tension_kn;
        stream >> max_compression_kn;
        stream >> max_abs_force_kn;
        stream >> overloaded_joints;
        stream >> broken_joints;
    }
};

//------------------------------------------------------------------------------
// ╨б╨╜╨╕╨╝╨╛╨║ ╨┤╨╕╨░╨│╨╜╨╛╤Б╤В╨╕╨║╨╕ ╨▓╤Б╨╡╤Е ╤Б╨╛╤Б╤В╨░╨▓╨╛╨▓ (╨╛╤В╨┐╤А╨░╨▓╨╗╤П╨╡╤В╤Б╤П ╤А╨░╨╖ ╨▓ 0.5 ╤Б)
//------------------------------------------------------------------------------
struct simulator_diagnostics_update_t final
{
    std::vector<simulator_train_diagnostics_t> trains;
    std::vector<simulator_vehicle_diagnostics_t> vehicles;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << static_cast<std::uint32_t>(trains.size());
        for (const auto& train : trains)
        {
            stream << train.serialize();
        }

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
        trains.clear();
        trains.resize(num);

        for (auto& train : trains)
        {
            QByteArray train_data;
            stream >> train_data;
            train.deserialize(train_data);
        }

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
struct simulator_train_profile_point_t final
{
    /// ╨Ф╨╕╤Б╤В╨░╨╜╤Ж╨╕╤П ╨╛╤В ╤Б╨╡╤А╨╡╨┤╨╕╨╜╤Л ╨┐╨╛╨╡╨╖╨┤╨░ ╨▓╨┤╨╛╨╗╤М ╨┐╤Г╤В╨╕, ╨╝ (╨▓╨┐╨╡╤А╤С╨┤ ╨┐╨╛ ╤Е╨╛╨┤╤Г - ┬л+┬╗, ╨╜╨░╨╖╨░╨┤ - ┬л-┬╗)
    float distance = 0.0f;

    /// ╨Т╤Л╤Б╨╛╤В╨░ ╨┐╤Г╤В╨╕, ╨╝
    float elevation = 0.0f;

    /// ╨Ц╨╡╨╗╨╡╨╖╨╜╨╛╨┤╨╛╤А╨╛╨╢╨╜╤Л╨╣ ╨┐╨╕╨║╨╡╤В╨░╨╢ ╨▓ ╤Н╤В╨╛╨╣ ╤В╨╛╤З╨║╨╡, ╨╝
    float railway_coord = 0.0f;

    /// ╨г╨║╨╗╨╛╨╜ ╨┐╤А╨╛╤Д╨╕╨╗╤П ╨╜╨░ ╤Б╨╡╨│╨╝╨╡╨╜╤В╨╡, ╨▓ ╤В╤Л╤Б╤П╤З╨╜╤Л╤Е
    float inclination = 0.0f;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << distance;
        stream << elevation;
        stream << railway_coord;
        stream << inclination;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> distance;
        stream >> elevation;
        stream >> railway_coord;
        stream >> inclination;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_train_profile_vehicle_t final
{
    /// ╨Ь╨╛╨┤╨╡╨╗╤М-╨╕╨╜╨┤╨╡╨║╤Б ╨Я╨Х, ╨╖╨░╨╜╨╕╨╝╨░╤О╤Й╨╡╨╣ ╤Г╤З╨░╤Б╤В╨╛╨║ ╨┐╤А╨╛╤Д╨╕╨╗╤П
    int vehicle_id = 0;

    /// ╨Э╨░╤З╨░╨╗╨╛ ╨╖╨░╨╜╨╕╨╝╨░╨╡╨╝╨╛╨│╨╛ ╨╕╨╜╤В╨╡╤А╨▓╨░╨╗╨░ ╨┐╨╛ ╨┤╨╕╤Б╤В╨░╨╜╤Ж╨╕╨╕ ╨┐╤А╨╛╤Д╨╕╨╗╤П, ╨╝
    float begin_distance = 0.0f;

    /// ╨Ъ╨╛╨╜╨╡╤Ж ╨╖╨░╨╜╨╕╨╝╨░╨╡╨╝╨╛╨│╨╛ ╨╕╨╜╤В╨╡╤А╨▓╨░╨╗╨░ ╨┐╨╛ ╨┤╨╕╤Б╤В╨░╨╜╤Ж╨╕╨╕ ╨┐╤А╨╛╤Д╨╕╨╗╤П, ╨╝
    float end_distance = 0.0f;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << vehicle_id;
        stream << begin_distance;
        stream << end_distance;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> vehicle_id;
        stream >> begin_distance;
        stream >> end_distance;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_train_profile_signal_t final
{
    /// ╨Ф╨╕╤Б╤В╨░╨╜╤Ж╨╕╤П ╤Б╨▓╨╡╤В╨╛╤Д╨╛╤А╨░ ╨╛╤В ╤Б╨╡╤А╨╡╨┤╨╕╨╜╤Л ╨┐╨╛╨╡╨╖╨┤╨░, ╨╝ (╨▓╨┐╨╡╤А╤С╨┤ ╨┐╨╛ ╤Е╨╛╨┤╤Г - ┬л+┬╗, ╨╜╨░╨╖╨░╨┤ - ┬л-┬╗)
    float distance = 0.0f;

    /// ╨Ш╨╝╤П ╨║╨╛╨╜╨╜╨╡╨║╤В╨╛╤А╨░ (╤Б╤В╤А╨╡╨╗╨║╨╕), ╨╜╨░ ╨║╨╛╤В╨╛╤А╨╛╨╝ ╤Г╤Б╤В╨░╨╜╨╛╨▓╨╗╨╡╨╜ ╤Б╨▓╨╡╤В╨╛╤Д╨╛╤А
    QString connector_name = "";

    /// ╨Э╨░╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╨╡ ╤Б╨▓╨╡╤В╨╛╤Д╨╛╤А╨░ ╨╛╤В╨╜╨╛╤Б╨╕╤В╨╡╨╗╤М╨╜╨╛ ╨║╨╛╨╜╨╜╨╡╨║╤В╨╛╤А╨░ (FWD=1, BWD=-1)
    std::int8_t signal_dir = 0;

    /// ╨б╨╕╨│╨╜╨░╨╗ ╨╜╨░╨┐╤А╨░╨▓╨╗╨╡╨╜ ╨┐╤А╨╛╤В╨╕╨▓ ╨┤╨▓╨╕╨╢╨╡╨╜╨╕╤П ╨┐╨╛╨╡╨╖╨┤╨░ (╨▓╤Б╨╡ ╨╗╨╕╨╜╨╖╤Л ╨┐╨╛╨│╨░╤И╨╡╨╜╤Л)
    bool is_oncoming = false;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << distance;
        stream << connector_name;
        stream << signal_dir;
        stream << is_oncoming;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> distance;
        stream >> connector_name;
        stream >> signal_dir;
        stream >> is_oncoming;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_train_profile_station_t final
{
    /// ╨Ф╨╕╤Б╤В╨░╨╜╤Ж╨╕╤П ╤Б╤В╨░╨╜╤Ж╨╕╨╕ ╨╛╤В ╤Б╨╡╤А╨╡╨┤╨╕╨╜╤Л ╨┐╨╛╨╡╨╖╨┤╨░, ╨╝ (╨▓╨┐╨╡╤А╤С╨┤ ╨┐╨╛ ╤Е╨╛╨┤╤Г - ┬л+┬╗, ╨╜╨░╨╖╨░╨┤ - ┬л-┬╗)
    float distance = 0.0f;

    /// ╨Э╨░╨╖╨▓╨░╨╜╨╕╨╡ ╤Б╤В╨░╨╜╤Ж╨╕╨╕
    QString name = "";

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << distance;
        stream << name;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> distance;
        stream >> name;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_train_profile_speed_limit_t final
{
    /// ╨Ф╨╕╤Б╤В╨░╨╜╤Ж╨╕╤П ╨╛╤В ╤Б╨╡╤А╨╡╨┤╨╕╨╜╤Л ╨┐╨╛╨╡╨╖╨┤╨░ ╨▓╨┤╨╛╨╗╤М ╨┐╤Г╤В╨╕, ╨╝ (╨▓╨┐╨╡╤А╤С╨┤ ╨┐╨╛ ╤Е╨╛╨┤╤Г - ┬л+┬╗, ╨╜╨░╨╖╨░╨┤ - ┬л-┬╗)
    float distance = 0.0f;

    /// ╨Ъ╨╛╨╜╨╡╤Ж ╨╕╨╜╤В╨╡╤А╨▓╨░╨╗╨░ ╨╛╨│╤А╨░╨╜╨╕╤З╨╡╨╜╨╕╤П, ╨╝
    float end_distance = 0.0f;

    /// ╨Ю╨│╤А╨░╨╜╨╕╤З╨╡╨╜╨╕╨╡ ╤Б╨║╨╛╤А╨╛╤Б╤В╨╕, ╨║╨╝/╤З
    float speed_kmh = 0.0f;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << distance;
        stream << end_distance;
        stream << speed_kmh;

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> distance;
        stream >> end_distance;
        stream >> speed_kmh;
    }
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct simulator_train_profile_update_t final
{
    /// ╨Ш╨╜╨┤╨╡╨║╤Б ╨┐╨╛╨╡╨╖╨┤╨░ ╨╜╨░ ╤В╨╡╨║╤Г╤Й╨╕╨╣ ╨║╨░╨┤╤А
    int train_id = 0;

    /// ╨Ь╨╛╨┤╨╡╨╗╤М-╨╕╨╜╨┤╨╡╨║╤Б ╤Б╤А╨╡╨┤╨╜╨╡╨╣ ╨Я╨Х - ╤В╨╛╤З╨║╨░ ╨╛╤В╤Б╤З╤С╤В╨░ ╨┐╤А╨╛╤Д╨╕╨╗╤П (distance = 0)
    int middle_vehicle_id = 0;

    /// ╨Э╨░╨┐╤А╨░╨▓╨╗╨╡╨╜╨╕╨╡ ╨┤╨▓╨╕╨╢╨╡╨╜╨╕╤П: +1 ╨▓╨┐╨╡╤А╤С╨┤ ╨┐╨╛ ╨┐╤А╨╛╤Д╨╕╨╗╤О, -1 ╨╜╨░╨╖╨░╨┤
    int direction = 1;

    /// ╨б╨║╨╛╤А╨╛╤Б╤В╤М ╨┐╨╛╨╡╨╖╨┤╨░, ╨╝/╤Б
    float speed = 0.0f;

    /// ╨д╨░╨║╤В╨╕╤З╨╡╤Б╨║╨╕╨╡ ╨┐╤А╨╛╤В╤П╨╢╤С╨╜╨╜╨╛╤Б╤В╨╕ ╨┐╤А╨╛╤Д╨╕╨╗╤П ╨╜╨░╨╖╨░╨┤ ╨╕ ╨▓╨┐╨╡╤А╤С╨┤, ╨╝
    float backward = 0.0f;
    float forward = 0.0f;

    /// ╨Ч╨░╨┐╤А╨╛╤И╨╡╨╜╨╜╤Л╨╡ ╨┐╤А╨╛╤В╤П╨╢╤С╨╜╨╜╨╛╤Б╤В╨╕ ╨┐╤А╨╛╤Д╨╕╨╗╤П ╨╜╨░╨╖╨░╨┤ ╨╕ ╨▓╨┐╨╡╤А╤С╨┤, ╨╝
    /// (╨│╨╛╤А╨╕╨╖╨╛╨╜╤В╨░╨╗╤М╨╜╤Л╨╣ ╨╝╨░╤Б╤И╤В╨░╨▒ ╨╛╤В╨╛╨▒╤А╨░╨╢╨╡╨╜╨╕╤П ╤Б╨╛╨╛╤В╨▓╨╡╤В╤Б╤В╨▓╤Г╨╡╤В ╨╕╨╝)
    float backward_requested = 0.0f;
    float forward_requested = 0.0f;

    /// ╨Т╨╡╤А╤И╨╕╨╜╤Л ╨╗╨╛╨╝╨░╨╜╨╛╨╣ ╨┐╤А╨╛╤Д╨╕╨╗╤П, ╤Г╨┐╨╛╤А╤П╨┤╨╛╤З╨╡╨╜╤Л ╨┐╨╛ distance ╨╛╤В -backward ╨┤╨╛ +forward
    std::vector<simulator_train_profile_point_t> profile;

    /// ╨Х╨┤╨╕╨╜╨╕╤Ж╤Л ╨┐╨╛╨┤╨▓╨╕╨╢╨╜╨╛╨│╨╛ ╤Б╨╛╤Б╤В╨░╨▓╨░, ╨╖╨░╨╜╨╕╨╝╨░╤О╤Й╨╕╨╡ ╤Г╤З╨░╤Б╤В╨║╨╕ ╨┐╤А╨╛╤Д╨╕╨╗╤П
    /// (╨▓╨║╨╗╤О╤З╨░╤П ╨▓╨░╨│╨╛╨╜╤Л ╨┤╤А╤Г╨│╨╕╤Е ╨┐╨╛╨╡╨╖╨┤╨╛╨▓), ╤Г╨┐╨╛╤А╤П╨┤╨╛╤З╨╡╨╜╤Л ╨┐╨╛ begin_distance
    std::vector<simulator_train_profile_vehicle_t> vehicles;

    /// ╨б╨▓╨╡╤В╨╛╤Д╨╛╤А╤Л ╨╜╨░ ╨┐╤А╨╛╤Д╨╕╨╗╨╡ (╨┐╨╛╨┐╤Г╤В╨╜╤Л╨╡ ╨┐╨╛ ╤Е╨╛╨┤╤Г ╨┤╨▓╨╕╨╢╨╡╨╜╨╕╤П ╨┐╨╛╨╡╨╖╨┤╨░),
    /// ╤Г╨┐╨╛╤А╤П╨┤╨╛╤З╨╡╨╜╤Л ╨┐╨╛ distance
    std::vector<simulator_train_profile_signal_t> signal_list;

    /// ╨б╤В╨░╨╜╤Ж╨╕╨╕ ╨╜╨░ ╨┐╤А╨╛╤Д╨╕╨╗╨╡, ╤Г╨┐╨╛╤А╤П╨┤╨╛╤З╨╡╨╜╤Л ╨┐╨╛ distance
    std::vector<simulator_train_profile_station_t> stations;

    /// ╨Ю╨│╤А╨░╨╜╨╕╤З╨╡╨╜╨╕╤П ╤Б╨║╨╛╤А╨╛╤Б╤В╨╕ ╨╜╨░ ╨┐╤А╨╛╤Д╨╕╨╗╨╡, ╤Г╨┐╨╛╤А╤П╨┤╨╛╤З╨╡╨╜╤Л ╨┐╨╛ distance
    std::vector<simulator_train_profile_speed_limit_t> speed_limits;

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        stream << train_id;
        stream << middle_vehicle_id;
        stream << direction;
        stream << speed;
        stream << backward;
        stream << forward;
        stream << backward_requested;
        stream << forward_requested;

        stream << static_cast<std::uint32_t>(profile.size());
        for (const auto& point : profile)
        {
            stream << point.serialize();
        }

        stream << static_cast<std::uint32_t>(vehicles.size());
        for (const auto& vehicle : vehicles)
        {
            stream << vehicle.serialize();
        }

        stream << static_cast<std::uint32_t>(signal_list.size());
        for (const auto& signal : signal_list)
        {
            stream << signal.serialize();
        }

        stream << static_cast<std::uint32_t>(stations.size());
        for (const auto& station : stations)
        {
            stream << station.serialize();
        }

        stream << static_cast<std::uint32_t>(speed_limits.size());
        for (const auto& sl : speed_limits)
        {
            stream << sl.serialize();
        }

        return data;
    }

    void deserialize(QByteArray& data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> train_id;
        stream >> middle_vehicle_id;
        stream >> direction;
        stream >> speed;
        stream >> backward;
        stream >> forward;
        stream >> backward_requested;
        stream >> forward_requested;

        std::uint32_t num = 0;
        stream >> num;

        profile.clear();
        profile.resize(num);

        for (auto& point : profile)
        {
            QByteArray point_data;
            stream >> point_data;

            point.deserialize(point_data);
        }

        stream >> num;

        vehicles.clear();
        vehicles.resize(num);

        for (auto& vehicle : vehicles)
        {
            QByteArray vehicle_data;
            stream >> vehicle_data;

            vehicle.deserialize(vehicle_data);
        }

        stream >> num;

        signal_list.clear();
        signal_list.resize(num);

        for (auto& signal : signal_list)
        {
            QByteArray signal_data;
            stream >> signal_data;

            signal.deserialize(signal_data);
        }

        stream >> num;

        stations.clear();
        stations.resize(num);

        for (auto& station : stations)
        {
            QByteArray station_data;
            stream >> station_data;

            station.deserialize(station_data);
        }

        stream >> num;

        speed_limits.clear();
        speed_limits.resize(num);

        for (auto& sl : speed_limits)
        {
            QByteArray sl_data;
            stream >> sl_data;

            sl.deserialize(sl_data);
        }
    }
};


#endif // SIMULATOR_UPDATE_STRUCT_H
