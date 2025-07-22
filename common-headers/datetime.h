#ifndef     RRS_DATE_TIME_H
#define     RRS_DATE_TIME_H

#include    <cstdint>
#include    <ctime>

#include    <QBuffer>
#include    <QByteArray>
#include    <QDataStream>

// Храним время суток в 10-тысячных долях секунды от полуночи
#define TIMEUNIT_MULTIPLIER         10000
#define TIMEUNIT_MULTIPLIER_DAY     (TIMEUNIT_MULTIPLIER * 60 * 60 * 24)
#define TIMEUNIT_MULTIPLIER_HOUR    (TIMEUNIT_MULTIPLIER * 60 * 60)
#define TIMEUNIT_MULTIPLIER_MIN     (TIMEUNIT_MULTIPLIER * 60)
#define TIMEUNIT_MULTIPLIER_SEC     (TIMEUNIT_MULTIPLIER)
#define TIMEUNIT_MULTIPLIER_MSEC    (TIMEUNIT_MULTIPLIER / 1000)

static constexpr std::uint8_t days_in_month_nleap[12] = {
    31, 28, 31,
    30, 31, 30,
    31, 31, 30,
    31, 30, 31
};

static constexpr std::uint8_t days_in_month_leap[12] = {
    31, 29, 31,
    30, 31, 30,
    31, 31, 30,
    31, 30, 31
};

//------------------------------------------------------------------------------
// Структура для хранения даты сервера RRS
//------------------------------------------------------------------------------
struct server_date_t final
{
private:
    union
    {
        std::int32_t date_data;
        struct
        {
            std::int16_t y;
            std::uint8_t m;
            std::uint8_t d;
        };
    };

public:
    server_date_t()
        : y(2000)
        , m(1)
        , d(1)
    {
    }

    server_date_t(std::int16_t year, std::uint8_t month, std::uint8_t day)
        : y(year)
        , m(month)
        , d(day)
    {
    }

    constexpr int32_t data() const
    {
        return date_data;
    }

    /// Год
    std::int16_t year() const noexcept
    {
        return y;
    }

    /// Месяц
    std::uint8_t month() const noexcept
    {
        return m;
    }

    /// День
    std::uint8_t day() const noexcept
    {
        return d;
    }

    /// Високосный год
    static bool isLeapYear(std::int16_t year) noexcept
    {
        return ((year % 4 == 0) && (year % 100 > 0)) || (year % 400 == 0);
    }

    /// Переход к следующему дню
    void nextDay() noexcept
    {
        const std::uint8_t* const days_in_month = isLeapYear(y) ? days_in_month_leap : days_in_month_nleap;

        if (d >= days_in_month[m - 1])
        {
            d = 1;
            if (m == 12) // Декабрь
            {
                m = 1;
                ++y;
            }
            else
            {
                ++m;
            }
        }
        else
        {
            ++d;
        }
    }

    /// Задать дату, по умолчанию из текущей системной
    static server_date_t dateNow(std::tm* std_tm_now = nullptr) noexcept
    {
        if (!std_tm_now)
        {
            const std::time_t system_time = std::time(nullptr);
            std_tm_now = std::localtime(&system_time);
        }
        server_date_t date{static_cast<std::int16_t>(std_tm_now->tm_year + 1900),
                           static_cast<std::uint8_t>(std_tm_now->tm_mon + 1),
                           static_cast<std::uint8_t>(std_tm_now->tm_mday)};
        return date;
    }

    QByteArray serialize() const
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << date_data;
        return buff.data();
    }

    void deserialize(QByteArray& data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> date_data;
    }
};

constexpr bool operator==(const server_date_t& lhs, const server_date_t& rhs)
{
    return lhs.data() == rhs.data();
}

constexpr bool operator!=(const server_date_t& lhs, const server_date_t& rhs)
{
    return lhs.data() != rhs.data();
}

constexpr bool operator>(const server_date_t& lhs, const server_date_t& rhs)
{
    return lhs.data() > rhs.data();
}

constexpr bool operator>=(const server_date_t& lhs, const server_date_t& rhs)
{
    return lhs.data() >= rhs.data();
}

constexpr bool operator<(const server_date_t& lhs, const server_date_t& rhs)
{
    return lhs.data() < rhs.data();
}

constexpr bool operator<=(const server_date_t& lhs, const server_date_t& rhs)
{
    return lhs.data() <= rhs.data();
}

//------------------------------------------------------------------------------
// Структура для хранения времени суток сервера RRS
//------------------------------------------------------------------------------
struct server_time_t final
{
private:
    std::uint32_t time_unit_since_midnight = 0;

public:
    server_time_t() noexcept = default;

    server_time_t(std::uint8_t hour, std::uint8_t minute, std::uint8_t sec, std::uint16_t msec = 0) noexcept
    {
        time_unit_since_midnight = TIMEUNIT_MULTIPLIER_MSEC * ((msec < 1000) ? msec : 999);
        time_unit_since_midnight += TIMEUNIT_MULTIPLIER_SEC * ((sec < 60) ? sec : 59);
        time_unit_since_midnight += TIMEUNIT_MULTIPLIER_MIN * ((minute < 60) ? minute : 59);
        time_unit_since_midnight += TIMEUNIT_MULTIPLIER_HOUR * ((hour < 24) ? hour : 23);
    }

    constexpr int32_t data() const
    {
        return time_unit_since_midnight;
    }

    /// Час
    std::uint8_t hour() const noexcept
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_HOUR;
    }

    /// Минута
    std::uint8_t minute() const noexcept
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_MIN % 60;
    }

    /// Секунда
    std::uint8_t sec() const noexcept
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_SEC % 60;
    }

    /// Миллисекунда
    std::uint16_t msec() const noexcept
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_MSEC % 1000;
    }

    /// Интегрирование времени, возвращает true если нужен переход к следующему дню
    bool addTime(double add_sec) noexcept
    {
        if (add_sec < 0.0)
        {
            return false;
        }

        const std::uint32_t add_timeunit = add_sec * TIMEUNIT_MULTIPLIER_SEC;
        time_unit_since_midnight += add_timeunit;

        if (time_unit_since_midnight < TIMEUNIT_MULTIPLIER_DAY)
        {
            return false;
        }

        time_unit_since_midnight = time_unit_since_midnight - TIMEUNIT_MULTIPLIER_DAY;
        return true;
    }

    /// Задать время, по умолчанию из текущего системного
    static server_time_t timeNow(std::tm* std_tm_now = nullptr) noexcept
    {
        if (!std_tm_now)
        {
            const std::time_t system_time = std::time(nullptr);
            std_tm_now = std::localtime(&system_time);
        }
        server_time_t time{static_cast<std::uint8_t>(std_tm_now->tm_hour),
                           static_cast<std::uint8_t>(std_tm_now->tm_min),
                           static_cast<std::uint8_t>(std_tm_now->tm_sec)};
        return time;
    }

    QByteArray serialize() const
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << time_unit_since_midnight;
        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> time_unit_since_midnight;
    }
};

constexpr bool operator==(const server_time_t& lhs, const server_time_t& rhs)
{
    return lhs.data() == rhs.data();
}

constexpr bool operator!=(const server_time_t& lhs, const server_time_t& rhs)
{
    return lhs.data() != rhs.data();
}

constexpr bool operator>(const server_time_t& lhs, const server_time_t& rhs)
{
    return lhs.data() > rhs.data();
}

constexpr bool operator>=(const server_time_t& lhs, const server_time_t& rhs)
{
    return lhs.data() >= rhs.data();
}

constexpr bool operator<(const server_time_t& lhs, const server_time_t& rhs)
{
    return lhs.data() < rhs.data();
}

constexpr bool operator<=(const server_time_t& lhs, const server_time_t& rhs)
{
    return lhs.data() <= rhs.data();
}

//------------------------------------------------------------------------------
// Структура для хранения времени симуляции сервера RRS
//------------------------------------------------------------------------------
struct simulator_time_t final
{
    server_date_t date;
    server_time_t time;
    double simulation_seconds = 0.0;

    simulator_time_t() noexcept = default;

    simulator_time_t(server_date_t in_date, server_time_t in_time, double in_simulation_seconds = 0.0) noexcept
        : date(in_date)
        , time(in_time)
        , simulation_seconds(in_simulation_seconds)
    {
    }

    /// Интегрирование времени
    void addTime(double add_sec) noexcept
    {
        simulation_seconds += add_sec;

        if (time.addTime(add_sec))
        {
            date.nextDay();
        }
    }

    /// Задать время, по умолчанию из текущего системного
    static simulator_time_t timeNow(std::tm* std_tm_now = nullptr) noexcept
    {
        if (!std_tm_now)
        {
            const std::time_t system_time = std::time(nullptr);
            std_tm_now = std::localtime(&system_time);
        }
        simulator_time_t sim_time{server_date_t::dateNow(std_tm_now),
                                  server_time_t::timeNow(std_tm_now)};
        return sim_time;
    }

    QByteArray serialize() const
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << date.serialize();

        stream << time.serialize();

        stream << simulation_seconds;

        return buff.data();
    }

    void deserialize(QByteArray& data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        QByteArray date_data;
        stream >> date_data;
        date.deserialize(date_data);

        QByteArray time_data;
        stream >> time_data;
        time.deserialize(time_data);

        stream >> simulation_seconds;
    }
};

#endif // RRS_DATE_TIME_H
