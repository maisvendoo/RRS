#ifndef     RRS_DATE_TIME_H
#define     RRS_DATE_TIME_H

#include    <cstdint>
#include    <ctime>

#include    <QByteArray>
#include    <QBuffer>
#include    <QDataStream>

// Храним время суток в 10-тысячных долях секунды от полуночи
#define TIMEUNIT_MULTIPLIER         10000
#define TIMEUNIT_MULTIPLIER_DAY     (TIMEUNIT_MULTIPLIER * 60 * 60 * 24)
#define TIMEUNIT_MULTIPLIER_HOUR    (TIMEUNIT_MULTIPLIER * 60 * 60)
#define TIMEUNIT_MULTIPLIER_MIN     (TIMEUNIT_MULTIPLIER * 60)
#define TIMEUNIT_MULTIPLIER_SEC     (TIMEUNIT_MULTIPLIER)
#define TIMEUNIT_MULTIPLIER_MSEC    (TIMEUNIT_MULTIPLIER / 1000)

//------------------------------------------------------------------------------
// Структура для хранения даты сервера RRS
//------------------------------------------------------------------------------
struct server_date_t final
{
private:
    union
    {
        int32_t date_data;
        struct
        {
            int16_t y;
            uint8_t m;
            uint8_t d;
        };
    };

public:
    server_date_t()
        : y(2000)
        , m(1)
        , d(1)
    {
    }

    server_date_t(int16_t year, uint8_t month, uint8_t day)
        : y(year)
        , m(month)
        , d(day)
    {
    }

    /// Год
    int16_t year() const
    {
        return y;
    }

    /// Месяц
    uint8_t month() const
    {
        return m;
    }

    /// День
    uint8_t day() const
    {
        return d;
    }

    /// Високосный год
    static bool isLeapYear(const int16_t& year)
    {
        return ((year % 4 == 0) && (year % 100 > 0)) || (year % 400 == 0);
    }

    /// Переход к следующему дню
    void nextDay()
    {
        // Декабрь
        if (m >= 12)
        {
            // 31-е - конец месяца и года
            if (d >= 31)
            {
                ++y;
                m = 1;
                d = 1;
                return;
            }
            ++d;
            return;
        }

        // Февраль
        if (m == 2)
        {
            // 28-е или 29-е - конец месяца
            if (isLeapYear(y) ? (d >= 29) : (d >= 28))
            {
                ++m;
                d = 1;
                return;
            }
            ++d;
            return;
        }

        // Апрель, июнь, сентябрь, ноябрь
        if ((m == 4) || (m == 6) || (m == 9) || (m == 11))
        {
            // 30-е - конец месяца
            if (d >= 30)
            {
                ++m;
                d = 1;
                return;
            }
            ++d;
            return;
        }

        // Январь, март, май, июль, август, октябрь
        // 31-е - конец месяца
        if (d >= 31)
        {
            ++m;
            d = 1;
            return;
        }
        ++d;
        return;
    }

    /// Задать дату, по умолчанию из текущей системной
    static server_date_t dateNow(std::tm* std_tm_now = nullptr)
    {
        if (!std_tm_now)
        {
            std::time_t system_time = std::time(nullptr);
            std_tm_now = std::localtime(&system_time);
        }
        server_date_t date{static_cast<int16_t>(std_tm_now->tm_year + 1900),
                           static_cast<uint8_t>(std_tm_now->tm_mon + 1),
                           static_cast<uint8_t>(std_tm_now->tm_mday)};
        return date;
    }

    QByteArray serialize()
    {
        QByteArray data;
        QBuffer buff(&data);
        buff.open(QIODevice::WriteOnly);
        QDataStream stream(&buff);

        stream << date_data;
        return buff.data();
    }

    void deserialize(QByteArray &data)
    {
        QBuffer buff(&data);
        buff.open(QIODevice::ReadOnly);
        QDataStream stream(&buff);

        stream >> date_data;
    }
};

//------------------------------------------------------------------------------
// Структура для хранения времени суток сервера RRS
//------------------------------------------------------------------------------
struct server_time_t final
{
private:
    uint32_t time_unit_since_midnight;

public:
    server_time_t()
        : time_unit_since_midnight(0)
    {
    }

    server_time_t(uint8_t hour, uint8_t minute, uint8_t sec, uint16_t msec = 0)
    {
        time_unit_since_midnight = TIMEUNIT_MULTIPLIER_MSEC * ((msec < 1000) ? msec : 999);
        time_unit_since_midnight += TIMEUNIT_MULTIPLIER_SEC * ((sec < 60) ? sec : 59);
        time_unit_since_midnight += TIMEUNIT_MULTIPLIER_MIN * ((minute < 60) ? minute : 59);
        time_unit_since_midnight += TIMEUNIT_MULTIPLIER_HOUR * ((hour < 24) ? hour : 23);
    }

    /// Час
    uint8_t hour() const
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_HOUR;
    }

    /// Минута
    uint8_t minute() const
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_MIN % 60;
    }

    /// Секунда
    uint8_t sec() const
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_SEC % 60;
    }

    /// Миллисекунда
    uint16_t msec() const
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_MSEC % 1000;
    }

    /// Интегрирование времени, возвращает true если нужен переход к следующему дню
    bool addTime(double add_sec)
    {
        if (add_sec < 0.0)
            return false;

        uint32_t add_timeunit = add_sec * TIMEUNIT_MULTIPLIER_SEC;
        time_unit_since_midnight += add_timeunit;

        if (time_unit_since_midnight < TIMEUNIT_MULTIPLIER_DAY)
            return false;

        time_unit_since_midnight = time_unit_since_midnight - TIMEUNIT_MULTIPLIER_DAY;
        return true;
    }

    /// Задать время, по умолчанию из текущего системного
    static server_time_t timeNow(std::tm* std_tm_now = nullptr)
    {
        if (!std_tm_now)
        {
            std::time_t system_time = std::time(nullptr);
            std_tm_now = std::localtime(&system_time);
        }
        server_time_t time{static_cast<uint8_t>(std_tm_now->tm_hour),
                           static_cast<uint8_t>(std_tm_now->tm_min),
                           static_cast<uint8_t>(std_tm_now->tm_sec)};
        return time;
    }

    QByteArray serialize()
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

//------------------------------------------------------------------------------
// Структура для хранения времени симуляции сервера RRS
//------------------------------------------------------------------------------
struct simulator_time_t final
{
    server_date_t date;
    server_time_t time;
    double simulation_seconds;

    simulator_time_t()
        : date(server_date_t())
        , time(server_time_t())
        , simulation_seconds(0.0)
    {
    }

    simulator_time_t(server_date_t in_date, server_time_t in_time, double in_simulation_seconds = 0.0)
        : date(in_date)
        , time(in_time)
        , simulation_seconds(in_simulation_seconds)
    {
    }

    /// Интегрирование времени
    void addTime(double add_sec)
    {
        simulation_seconds += add_sec;

        if (time.addTime(add_sec))
            date.nextDay();
    }

    /// Задать время, по умолчанию из текущего системного
    static simulator_time_t timeNow(std::tm* std_tm_now = nullptr)
    {
        if (!std_tm_now)
        {
            std::time_t system_time = std::time(nullptr);
            std_tm_now = std::localtime(&system_time);
        }
        simulator_time_t sim_time{server_date_t::dateNow(std_tm_now),
                                  server_time_t::timeNow(std_tm_now)};
        return sim_time;
    }

    QByteArray serialize()
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

    void deserialize(QByteArray &data)
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
