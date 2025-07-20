#ifndef     RRS_DATE_TIME_H
#define     RRS_DATE_TIME_H

#include    <cstdint>

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
            // 28-е - конец месяца
            if (d >= 28)
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

#endif // RRS_DATE_TIME_H
