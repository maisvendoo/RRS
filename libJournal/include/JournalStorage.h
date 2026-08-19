#ifndef JOURNALSTORAGE_H_
#define JOURNALSTORAGE_H_

#include <QString>
#include <QDateTime>


/**
 * @brief Уровни сообщений и функции выделения сообщений по уровню
 */
struct JournalLevel {
    enum Level {            // Уровни сообщений
        None            =   0x00,   // Нет сообщений (0)
        Critical        =   0x01,   // Критические события (1)
        Error           =   0x02,   // Ошибки (2)
        Warning         =   0x04,   // Предупреждения (4)
        Message         =   0x08,   // Важные рабочие условия (8)
        Info            =   0x10,   // Информационные сообщения (16)
        Debug           =   0x20,   // Отладочное сообщение (32)
        Trace           =   0x40,   // Сообщение на каждый чих (64)
        TraceCalls      =   0x80,   // Отслеживать вызовы функций
        TrackRuntime    = 0x100,  // Отслеживать время выполнения функций функций
        TrackParameters = 0x200,

        All = None        // Все вместе
            | Critical
            | Error
            | Warning
            | Message
            | Info
            | Debug
            | Trace

    };

    static const unsigned int defaultLevel = 0xF; // Уровень вывода по умолчанию

    /**
     * @brief Статическая функция возвращает в C++-строке текстовое представление (маркер) уровня
     *
     * @param level уровень сообщения
     *
     * @return C++-строка, текстовое представление (маркер) уровня
     */
    static QString printable(Level level)
    {
        switch(level)
        {
        case Critical: return "C";
        case Error: return "E";
        case Warning: return "W";
        case Message: return "M";
        case Info: return "I";
        case Debug: return "D";
        case Trace: return "T";
        case TraceCalls: return "S";
        case TrackRuntime: return "R";
        case TrackParameters: return "P";
        default: return "?";
        }
    }





// Escape sequence console color

/******************************************************************************************************************************

    Краткая дока по escape последовательностей для цвета в консоле

    Подробно за escape sequence Читать тут http://en.wikipedia.org/wiki/ANSI_escape_code


    "\033[" - Начало последовательности (CSI)

    CSI n [;k] m - Sets SGR (Select Graphic Rendition) parameters.
            After CSI can be zero or more parameters separated with ;.
            With no parameters, CSI m is treated as CSI 0 m (reset / normal),
            which is typical of most of the ANSI escape sequences.



     Color table
     Intensity  |  0    |  1    |  2    |   3   |  4    |  5    |  6    |  7    |  9
     ------------------------------------------------------------------------------------------
     Normal     |Black  |Red    |Green  |Yellow |Blue   |Magenta|Cyan   |White  |reset
     Bright     |Black  |Red    |Green  |Yellow |Blue   |Magenta|Cyan   |White  |

    SGR (Select Graphic Rendition) parameters

    Code    Effect                              Note
    ---------------------------------------------------------
    0       Reset / Normal                              all attributes off
    1       Intensity: Bold
    2       Intensity: Faint                            not widely supported
    3       Italic: on                                  not widely supported. Sometimes treated as inverse.
    4       Underline: Single                           not widely supported
    5       Blink: Slow                                 less than 150 per minute
    6       Blink: Rapid                                MS-DOS ANSI.SYS; 150 per minute or more
    7       Image: Negative                             inverse or reverse; swap foreground and background
    8       Conceal                                     not widely supported
    21      Underline: Double
    22      Intensity: Norma        l                   not bold and not faint
    24      Underline: None
    25      Blink: off
    27      Image: Positive
    28      Reveal                                      conceal off
    30–39     Set foreground color, normal intensity      3x, where x is from the color table above
    40–49     Set background color, normal intensity      4x, where x is from the color table above
    90–99     Set foreground color, high intensity        aixterm
    100–109 set background color, high intensity      aixterm

******************************************************************************************************************************/

    /**
     * @brief Статическая функция возвращает C++-строку, начинающую выделение сообщения цветом, соответствующим
     * уровню
     *
     * @param level уровень сообщения
     *
     * @return C++-строка, открывающая выделение цветом
     */
    static QString printableColor(Level level)
    {
        switch(level)
        {
        case Critical: return "\033[30;41m"+printable(level);// Яркий красный фон, черные буквы
        case Error: return "\033[30;45m"+printable(level);   // Яркий пурпурный фон, черные буквы
        case Warning: return "\033[30;43m"+printable(level); // Яркий желтый фон, черные буквы
        case Message: return "\033[32m"+printable(level);      // Зеленые буквы
        case Info: return "\033[0m"+printable(level);         // Нет цвета
        case Debug: return "\033[33m"+printable(level);       // Яркие желтые буквы
        case Trace: return "\033[36m"+printable(level);       // Яркие Cyan буквы
        default: return "\033[0m"+printable(level);           // Цвет: none
        }
    }

    /**
     * @brief Статическая функция возвращает C++-строку, закрывающую выделение цветом
     *
     * @return C++-строка, закрывающая выделение цветом
     */
    static QString endColor()
    {
        return "\033[0m"; // Конец выделения цветом
    }
};

Q_DECLARE_FLAGS(JournalLevels, JournalLevel::Level)

/**
 * @brief Базовый класс для журнальных хранилищ
 */
class JournalStorage
{
public:
    /**
     * @brief Конструктор
     *
     * @param level маска уровней хранилища
     */
    JournalStorage(unsigned int level);

    /**
     * @brief Деструктор
     */
    virtual ~JournalStorage();

    /**
     * @brief Процедура выводит сообщение на консоль или стандартное устрйство вывода ошибок
     *
     * @param time время, указываемое при записи сообщения
     * @param level уровень сообщения
     * @param record C++-строка сообщения
     *
     * Если маска хранилища не имеет совпадений с уровнем сообщения, возвращается.
     * Если уровень сообщения - ошибка или критическая ошибка, выводит выделенное цветом сообщение
     * с указанным временем на стандартное устройство вывода ошибок, иначе - на консоль.
     */
    virtual void write( const QDateTime& time, JournalLevel::Level level, const QString& record );

    /**
     * @brief Метод возвращает маску уровней хранилища (значение поля)
     *
     * @return маска уровней хранилища
     */
    unsigned int level() const;

private:
    unsigned int m_level;   // маска уровней хранилища

};



#endif /* JOURNALSTORAGE_H_ */
