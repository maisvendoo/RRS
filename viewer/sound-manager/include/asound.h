//-----------------------------------------------------------------------------
//
//      Библиотека для работы с 3D звуком
//      (c) РГУПС, ВЖД 24/03/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Библиотека для работы с 3D звуком
 *  \copyright РГУПС, ВЖД
 *  \author Ковшиков С. В.
 *  \date 24/03/2017
 */

#ifndef ASOUND_H
#define ASOUND_H

#include <QObject>
#include <QMap>
#include <AL/al.h>
#include <AL/alc.h>

#include "asound-log.h"

class QFile;
class QTimer;

#define BUFFER_BLOCKS 3
//-----------------------------------------------------------------------------
// Класс ASound
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
/*!
 * \struct wave_info_header_t
 * \brief Структура для хранения секции RIFF & WAVE файла
 */
struct wave_info_header_t
{
    char            chunkId[4];     ///< ID главного фрагмента "RIFF"
    uint32_t        chunkSize;      ///< Размер первого фрагмента
    char            format[4];      ///< Формат "WAVE"
    wave_info_header_t()
    {
        strcpy(chunkId, "");
        chunkSize = 0;
        strcpy(format, "");
    }
};
/*!
 * \struct wave_info_t
 * \brief Структура для хранения данных о wav файле
 */
struct wave_info_fmt_t
{

    char            subchunk1Id[4]; ///< ID первого подфрагмента "fmt"
    uint32_t        subchunk1Size;  ///< Размер первого подфрагмента
    short           audioFormat;    ///< Формат сжатия
    short           numChannels;    ///< Количество каналов
    uint32_t        sampleRate;     ///< Частота дискретизации (frequency)
    uint32_t        byteRate;       ///< Байт в секунду
    short           bytesPerSample; ///< Байт в одном сэмпле (blockAlign)
    short           bitsPerSample;  ///< Бит в сэмпле
// Constructor
    wave_info_fmt_t()
    {
        strcpy(subchunk1Id, "");
        subchunk1Size = 0;
        audioFormat = 0;
        numChannels = 0;
        sampleRate = 0;
        byteRate = 0;
        bytesPerSample = 0;
        bitsPerSample = 0;
    }
};

/*!
 * \struct wave_info_file_data_t
 * \brief Структура для хранения данных "data" WAVE файла
 */
struct wave_info_file_data_t
{
    char            subchunk2Id[4]; ///< ID второго субфрагмента "data"
    uint32_t        subchunk2Size;  ///< Размер дорожки
// Constructor
    wave_info_file_data_t()
    {
        strcpy(subchunk2Id, "");
        subchunk2Size = 0;
    }
};

/*!
 * \struct wave_cue_head_t
 * \brief Структура для хранения "шапки" фрагмента CUE
 */
struct wave_cue_head_t
{
    char            cueChunckId[4]; ///< ID фрагмента CUE (4 байта) "0x63756520"
    uint32_t        cueChunckSize;  ///< Размер фрагмента CUE (4 байта)
    uint32_t        cueChunckPNum;  ///< Кол-во точек в CUE списке (4 байта)
// Конструктор
    wave_cue_head_t()
    {
        strcpy(cueChunckId, "");
        cueChunckSize = 0;
        cueChunckPNum = 0;
    }
};

/*!
 * \struct wave_cue_data_t
 * \brief Структура для хранения данных фрагмента CUE
 */
struct wave_cue_data_t
{
    int32_t         ID;             ///< Уникальный идентификатор cue точки
    uint32_t        position;       ///< Смещение выборки связанной с точкой cue
    char            dataChunckId[4];///< "data"
    uint32_t        chunckStart;    ///< Байтовое смещение в секции списка WAVE
    uint32_t        blockStart;     ///< Смещение в секции data (начало блока)
    uint32_t        sampleOffset;   ///< Смещение выборки в секцию data
// Конструктор
    wave_cue_data_t()
    {
        ID = 0;
        position = 0;
        strcpy(dataChunckId, "");
        chunckStart = 0;
        blockStart = 0;
        sampleOffset = 0;
    }
};

/*!
 * \struct wave_list_head_t
 * \brief Структура для хранения данных "шапки" фрагмента LIST
 */
struct wave_list_head_t
{
    char            chunckId[4];    ///< "LIST" или "list"
    uint32_t        dataSize;       ///< Размер фрагмента LIST
    char            typeID[4];      ///< ID связанного типа данных "adtl"
// Конструктор
    wave_list_head_t()
    {
        strcpy(chunckId, "");
        dataSize = 0;
        strcpy(typeID, "");
    }
};
#pragma pack(pop)


/// Скорость воспроизведения источника по умолчанию
const float DEF_SRC_PITCH = 1.0f;

/// Минимальная громкость источника
const int MIN_SRC_VOLUME  = 0.0f;
/// Громкость источника по умолчанию
const int DEF_SRC_VOLUME  = 1.0f;
/// Максимальная громкость источника
const int MAX_SRC_VOLUME  = 1.0f;

/// Положение источника по умолчанию
const float DEF_SRC_POS[3] = {0.0f, 0.0f, 1.0f};
/// "Скорость передвижения" источника по умолчанию
const float DEF_SRC_VEL[3] = {0.0f, 0.0f, 0.0f};

/*!
 * \class ASound
 * \brief Класс реализующий создание источника звука, настройки его
 * пространственных характеристик, загрузки аудиофайла в формате wav и
 * последующего воспроизведения
 */
class ASound : public QObject
{
    Q_OBJECT
    Q_PROPERTY(std::string lastError_ WRITE setLastError NOTIFY lastErrorChanged_)
public:
    /*!
     * \brief Конструктор
     * \param soundname - имя аудиофайла
     */
    ASound(QString soundname, LogFileHandler *log, QObject* parent = Q_NULLPTR);
    /// Деструктор
    ~ASound();

    /// Вернуть громкость
    float getVolume();

    /// Вернуть скорость воспроизведения
    float getPitch();

    /// Вернуть флаг зацикливания
    bool getLoop();

    /// Вернуть положение источника
    void getPosition(float &x, float &y, float &z);

    /// Вернуть "скорость передвижения" источника
    void getVelocity(float &x, float &y, float &z);

    /// Вернуть последнюю ошибку
    QString getLastError();

    /// Играет ли звук
    bool isPlaying();

    /// Остановлен ли звук
    bool isStopped();

    /// Приостановлен ли звук
    bool isPaused();

    /// Длительность звука (или i-го блока) в мс
    int getDuration(int idx = -1);

    void setLastError(const std::string& value)
    {
        LastError_ = "E - " + QString::fromStdString(value);
        emit lastErrorChanged_(value);
    }


public slots:
    /// Установить громкость
    void setVolume(float volume = 1.0f);

    /// Установить скорости воспроизведения
    void setPitch(float pitch);

    /// Установить зацикливание
    void setLoop(bool loop);

    /// Установить положение
    void setPosition(float x, float y, float z);

    /// Установить "скорость передвижения"
    void setVelocity(float x, float y, float z);

    /// Играть звук
    void play();

    /// Приостановить звук
    void pause();

    /// Остановить звук
    void stop();

signals:

    void lastErrorChanged_(const std::string);

    void notify(const std::string msg);


private slots:

    /// Слот обработки таймера для работы с циклом
    void onTimerLoopControl();

private:

    // Можно продолжать работу с файлом
    bool canDo_; ///< Флаг допуска к работе с файлом

    // Можно играть звук
    bool canPlay_; ///< Флаг допуска к воспроизведению звука

    // Имеет-ли файл секцию CUE
    bool canCUE_; ///< Флаг наличия фрагмента CUE

    // Имеет-ли файл секцию LABL
    bool canLABL_; ///< Флаг наличия меток в файле

    // Размер чанка блока date при квази-потоковом воспроизведении
    ALsizei DATA_CHUNK_SIZE;

    // Имя звука
    QString soundName_; ///< Имя файла

    // Последняя ошибка
    QString lastError_; ///< Текс последней ошибки

    // Переменная для хранения файла
    QFile* file_; ///< Контейнер файла

    // Информация формата входного звукового файла
    wave_info_header_t wave_info_header_; ///< Структура информации формата файла [RIFF&&WAVE]

    // Информация о файле .wav
    wave_info_fmt_t wave_info_; ///< Структура информации о файле

    // Секция data в WAVE файле
    wave_info_file_data_t wave_info_file_data_; ///< Структура информации секции data

    // "шапка" списка CUE
    wave_cue_head_t cue_head_; ///< Структура "шапка" CUE

    // Список меток CUE
    QList <wave_cue_data_t>cue_data_; ///< Структура информации списка CUE

    // "шапка" списка меток
    wave_list_head_t list_head_; ///< Структура "шапка" LIST

    // Список меток labels (имя, смещение в секции data)
    QMap<QString, uint64_t> wave_labels_; ///< Список меток

    // Хранилище для data секции (самой музыки) файла .wav
    unsigned char* wavData_[BUFFER_BLOCKS]; ///< Контейнер секций блока данных файла wav

    // Размер каждого из 3-х блоков данных файла .wav
    uint64_t blockSize_[BUFFER_BLOCKS]; ///< Размер блоков данных файла wav

    // Продолжительность каждого из 3-х блоков данных файла .wav в мс
    uint64_t blockDuration_[BUFFER_BLOCKS]; ///< Продолжительность блоков данных файла wav

    // Буфер OpenAL
    ALuint  buffer_[BUFFER_BLOCKS]; ///< Буфер OpenAL 3 секции (старт, цикл, остановка)

    // Необходимое число блоков зацикленной части звука, чтобы играть хотя бы секунду
    uint64_t num_cycle_blocks_; ///< Число блоков зацикленной части звука

    // Источник OpenAL
    ALuint  source_; ///< Источник OpenAL

    // Формат аудио (mono8/16 - stereo8/16) OpenAL
    ALenum  format_; ///< Формат аудио (mono8/16 - stereo8/16) OpenAL

    // Громкость
    ALfloat sourceVolume_; ///< Громкость

    // Скорости воспроизведения
    ALfloat sourcePitch_; ///< Скорость воспроизведения

    // Флаг зацикливания
    bool sourceLoop_; ///< Флаг зацикливания

    // Положение источника
    ALfloat sourcePosition_[3]; ///< Положение источника

    // "Скорость передвижения" источника
    ALfloat sourceVelocity_[3]; ///< "Скорость передвижения" источника

    /// Таймер для управления зацикленным звуком
    QTimer* timerControl_;

    /// Last error in asound
    QString LastError_;

    /// Полная подготовка файла
    void loadSound_(QString soundname);

    /// Загрузка файла (в т.ч. из ресурсов)
    void loadFile_(QString soundname);

    /// Чтение информации о файле .wav
    void readWaveInfo_();

    /// Чтение формата файла
    void readWaveHeader_();

    /// Чтение данных формата и секции data
    void readWaveFmtData_(QByteArray arr);

    /// Чтение фрагмента LIST ("шапки")
    void readWaveListChunckHeader_(QByteArray &baseStr);

    /// Определение формата аудио (mono8/16 - stereo8/16)
    void defineFormat_();

    /// Получение CUE фрагмента
    void getCUE_(QByteArray &baseStr);

    /// Получение списка меток (Labels)
    void getLabels_(QByteArray &baseStr);

    /// Вычисление продолжительности звука
    void calcDuration_();

    /// Генерация буфера и источника
    void generateStuff_();

    /// Настройка источника
    void configureSource_();

    /// Метод проверки необходимых параметров
    void checkValue(std::string baseStr, const char targStr[], QString err);

    /// Подпрограмма очистки контейнеров данных дорожки
    void deleteWAVEDataContainers();
};

#endif // ASOUND_H
