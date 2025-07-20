#ifndef     RRS_DATE_TIME_H
#define     RRS_DATE_TIME_H

#include    <cstdint>

// Храним время суток в 10-тысячных долях секунды от полуночи
#define TIMEUNIT_MULTIPLIER         10000
#define TIMEUNIT_MULTIPLIER_DAY     TIMEUNIT_MULTIPLIER * 60 * 60 * 24
#define TIMEUNIT_MULTIPLIER_HOUR    TIMEUNIT_MULTIPLIER * 60 * 60
#define TIMEUNIT_MULTIPLIER_MIN     TIMEUNIT_MULTIPLIER * 60
#define TIMEUNIT_MULTIPLIER_SEC     TIMEUNIT_MULTIPLIER
#define TIMEUNIT_MULTIPLIER_MSEC    TIMEUNIT_MULTIPLIER / 1000

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

    uint8_t hour() const
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_HOUR;
    }

    uint8_t minute() const
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_MIN % 60;
    }

    uint8_t sec() const
    {
        return time_unit_since_midnight / TIMEUNIT_MULTIPLIER_SEC % 60;
    }

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
};

#endif // RRS_DATE_TIME_H
