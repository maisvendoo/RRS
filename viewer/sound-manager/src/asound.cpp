//-----------------------------------------------------------------------------
//
//      Библиотека для работы со звуком
//      (c) РГУПС, ВЖД 24/03/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------


#include "asound.h"
#include "CfgReader.h"
#include "asound-log.h"
#include "filesystem.h"
#include <QTimer>

// ****************************************************************************
// *                         Класс AListener                                  *
// ****************************************************************************
//-----------------------------------------------------------------------------
// КОНСТРУКТОР
//-----------------------------------------------------------------------------
AListener::AListener()
{
    // Загрузка конфиг-файла
    double tmp_volume = 1.0;
    int tmp_max_sources = 65535;

    FileSystem &fs = FileSystem::getInstance();
    QString cfg_path = QString(fs.getConfigDir().c_str()) + fs.separator() + "sound-settings.xml";
    CfgReader cfg;

    if (cfg.load(cfg_path))
    {
        QString secName = "Settings";

        cfg.getDouble(secName, "Volume", tmp_volume);
        cfg.getInt(secName, "MaxSources", tmp_max_sources);
    }
    ALfloat volume = static_cast<ALfloat>( std::max( 0.0, std::min(1.0, tmp_volume) ) );
    ALCint max_sources = static_cast<ALCint>( std::max( 1, std::min(65535, tmp_max_sources) ) );

    // Открываем устройство
    device_ = alcOpenDevice(nullptr);
    // Создаём контекст
    ALCint context_atrribute_list[2] = {ALC_MONO_SOURCES, max_sources};
    context_ = alcCreateContext(device_, context_atrribute_list);
    // Устанавливаем текущий контекст
    alcMakeContextCurrent(context_);

    // Инициализируем положение слушателя
    memcpy(listenerPosition_,    DEF_LSN_POS, 3 * sizeof(float));
    // Инициализируем вектор "скорости передвижения" слушателя
    memcpy(listenerVelocity_,    DEF_LSN_VEL, 3 * sizeof(float));
    // Инициализируем векторы направления слушателя
    memcpy(listenerOrientation_, DEF_LSN_ORI, 6 * sizeof(float));

    // Устанавливаем положение слушателя
    alListenerfv(AL_POSITION,    listenerPosition_);
    // Устанавливаем скорость слушателя
    alListenerfv(AL_VELOCITY,    listenerVelocity_);
    // Устанавливаем направление слушателя
    alListenerfv(AL_ORIENTATION, listenerOrientation_);
    // Устанавливаем общий уровень громкости
    alListenerf(AL_GAIN, volume);

    log_ = new LogFileHandler("asound.log");
    log_->notify(QString("Volume: %1").arg(volume, 5, 'f', 3).toStdString());
    log_->notify(QString("Sources: %1").arg(max_sources).toStdString());
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AListener& AListener::getInstance()
{
    // Создаем статичный экземпляр класса
    static AListener instance;
    // Возвращаем его при каждом вызове метода
    return instance;
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AListener::closeDevices()
{
    alcDestroyContext(context_);
    alcCloseDevice(device_);
}



// ****************************************************************************
// *                            Класс ASound                                  *
// ****************************************************************************
//-----------------------------------------------------------------------------
// КОНСТРУКТОР
//-----------------------------------------------------------------------------
ASound::ASound(QString soundname, QObject *parent): QObject(parent),
    canDo_(false),              // Сбрасываем флаг
    canPlay_(false),            // Сбрасываем флаг
    soundName_(soundname),      // Сохраняем название звука
    source_(0),                 // Обнуляем источник
    format_(0),                 // Обнуляем формат
    sourceVolume_(DEF_SRC_VOLUME),  // Громкость по умолч.
    sourcePitch_(DEF_SRC_PITCH),    // Скорость воспроизведения по умолч.
    sourceLoop_(false)         // Зацикливание по-умолч.
{ 
    // Инициализируем позицию источника
    memcpy(sourcePosition_, DEF_SRC_POS, 3 * sizeof(float));
    // Инициализируем вектор "скорости передвижения" источника
    memcpy(sourceVelocity_, DEF_SRC_VEL, 3 * sizeof(float));
    // Создаём контейнер аудиофайла
    file_ = new QFile();

    // Зануляем все буферы и блоки данных
    for (int i = 0; i < BUFFER_BLOCKS; ++i)
    {
        wavData_[i] = nullptr;
        blockSize_[i] = 0;
        blockDuration_[i] = 0;
        buffer_[i] = 0;
    }
    num_cycle_blocks_ = 0;

    connect(this, &ASound::notify, AListener::getInstance().log_, &LogFileHandler::notify);
    connect(this, &ASound::lastErrorChanged_, AListener::getInstance().log_, &LogFileHandler::notify);

    emit notify("T Load sound: " + soundname.toStdString());

    // Загружаем звук
    loadSound_(soundname);

    timerControl_ = Q_NULLPTR;
}



//-----------------------------------------------------------------------------
// ДЕСТРУКТОР
//-----------------------------------------------------------------------------
ASound::~ASound()
{
    // Удаляем контейнеры данных
    deleteWAVEDataContainers();

    // Удаляем источник
    alDeleteSources(1, &source_);
    // Удаляем буфер
    alDeleteBuffers(BUFFER_BLOCKS, buffer_);
}



//-----------------------------------------------------------------------------
// Очистка контейнеров данных дорожки
//-----------------------------------------------------------------------------
void ASound::deleteWAVEDataContainers()
{
    for (int i = 0; i < BUFFER_BLOCKS; ++i)
    {
        if (wavData_[i])
            delete wavData_[i];
    }
}



//-----------------------------------------------------------------------------
// Полная подготовка файла
//-----------------------------------------------------------------------------
void ASound::loadSound_(QString soundname)
{
    // Сбрасываем флаги
    canDo_ = false;
    canPlay_ = false;
    canCUE_ = false;
    canLABL_ = false;

    // Сохраняем название звука
    soundName_ = soundname;

    // Загружаем файл
    loadFile_(soundname);

    // Читаем информационный раздел 44байта
    readWaveInfo_();

    // Определяем формат аудио (mono8/16 - stereo8/16) OpenAL
    defineFormat_();

    // Генерируем буфер и источник
    generateStuff_();

    // Настраиваем источник
    configureSource_();

    // Можно играть звук
    if (canDo_)
    {
        canPlay_ = true;
    }
}



//-----------------------------------------------------------------------------
// Загрузка файла (в т.ч. из ресурсов)
//-----------------------------------------------------------------------------
void ASound::loadFile_(QString soundname)
{
    // Загружаем файл в контейнер
    file_->setFileName(soundname);

    // Проверяем, существует ли файл
    if (!file_->exists())
    {
        setLastError("NO_SUCH_FILE: " + soundname.toStdString());
        lastError_ = "NO_SUCH_FILE: ";
        lastError_.append(soundname);
        canDo_ = false;
        return;
    }

    // Пытаемся открыть файл
    if (file_->open(QIODevice::ReadOnly))
    {
        canDo_ = true;
    }
    else
    {
        canDo_ = false;
        lastError_ = "CANT_OPEN_FILE_FOR_READING: ";
        lastError_.append(soundname);
        return;
    }
}



//-----------------------------------------------------------------------------
// Чтение информации о файле .wav
//-----------------------------------------------------------------------------
void ASound::readWaveInfo_()
{
    if (canDo_)
    {
        // Если ранее был загружен другой файл
        deleteWAVEDataContainers();

        // Читаем первые 12 байт файла
        readWaveHeader_();

        // Следующие 4 байта
        QByteArray arr = file_->read(4);

        if (strncmp(arr.data(), "JUNK", 4) == 0)
        {
            // Читаем длину сегмента JUNK
            arr = file_->read(4);
            int JUNKLen = arr.at(0);
            // Читаем данные блока JUNK в "никуда"
            file_->read(JUNKLen);
        }

        readWaveFmtData_(arr);

        if (canDo_)
        {
            // Читаем из файла сами медиа данные зная их размер
            arr = file_->read(wave_info_file_data_.subchunk2Size);
            // Читаем оставшуюся информацию из WAVE файла
            uint32_t extraBlockSize =
                    static_cast<uint32_t>(file_->size()) - wave_info_file_data_.subchunk2Size - sizeof(wave_info_fmt_t);
            QByteArray arrDop = file_->read(extraBlockSize);

            getCUE_(arrDop);

            if (canCUE_)
                getLabels_(arrDop);

            // Итератор для data и сдвиг точки копирования в блоке данных звука
            int32_t i = 0, data_offset = 0;
            // Массив байтов текущего блока для копирования в data
            QByteArray blockData;
            // Если присутствуют метки - грузим их в три буфера
            if (canLABL_)
            {
                QMap<QString, uint64_t>::const_iterator labl_map = wave_labels_.constBegin();
                while (labl_map != wave_labels_.constEnd()) {
                    if (labl_map.key() == "loop" || labl_map.key() == "stop")
                    {
                        blockSize_[i] = labl_map.value() - static_cast<uint64_t>(data_offset);
                        wavData_[i] = new unsigned char[blockSize_[i]];
                        blockData = arr.mid(data_offset, static_cast<int32_t>(labl_map.value()));
                        memcpy(wavData_[i], blockData.data(),
                               blockSize_[i]);
                        data_offset += blockSize_[i];
                        ++i;
                    }
                    ++labl_map;
                }
            }

            // Создаем массив для данных
            blockSize_[i] = wave_info_file_data_.subchunk2Size - static_cast<uint32_t>(data_offset);
            wavData_[i] = new unsigned char[blockSize_[i]];
            blockData = arr.mid(data_offset, static_cast<int>(blockSize_[i]));
            // Переносим данные в массив
            memcpy(wavData_[i], blockData.data(),
                   blockSize_[i]);

            calcDuration_();

            emit notify("| - File size: " + QString::number(file_->size()).toStdString());
            emit notify("| - File data size: " + QString::number(wave_info_file_data_.subchunk2Size).toStdString());
            emit notify("| - Byterate: " + QString::number(wave_info_.byteRate).toStdString());
            emit notify("| - Sample rate: " + QString::number(wave_info_.sampleRate).toStdString());
            emit notify("| - Num channels: " + QString::number(wave_info_.numChannels).toStdString());
            emit notify("| - Bits per sample: " + QString::number(wave_info_.bitsPerSample).toStdString());
            emit notify("| - Bytes per sample: " + QString::number(wave_info_.bytesPerSample).toStdString());
            emit notify("| - Buffer blocks: " + QString::number(i).toStdString());

            for (int i = 0; i < BUFFER_BLOCKS; ++i)
            {
                emit notify("| - Block #" + QString::number(i).toStdString() +
                            " size: " + QString::number(blockSize_[i]).toStdString());
            }

            for (int i = 0; i < BUFFER_BLOCKS; ++i)
            {
                emit notify("| - Block #" + QString::number(i).toStdString() +
                            " duration: " + QString::number(blockDuration_[i]).toStdString());
            }

            // Закрываем файл
            file_->close();
        }
    }
}


//-----------------------------------------------------------------------------
// Получение первых 12-и байт WAVE файла
//-----------------------------------------------------------------------------
void ASound::readWaveHeader_()
{
    // Читаем 12 байт информации о формате
    QByteArray arr = file_->read(sizeof(wave_info_header_));

    // Переносим все значения из массива в струтуру
    memcpy(&wave_info_header_, arr.data(),
           sizeof(wave_info_header_t));
    // Проверка данных формата
    checkValue(wave_info_header_.chunkId, "RIFF", "NOT_RIFF_FILE");
    checkValue(wave_info_header_.format, "WAVE", "NOT_WAVE_FILE");
}


//-----------------------------------------------------------------------------
// Получение данных о формате файла и секции data
//-----------------------------------------------------------------------------
void ASound::readWaveFmtData_(QByteArray arr)
{
    //QByteArray arr;
    // Ищем секцию data, откидывая все "ненужное" (PAD Sectors)
    while (!file_->atEnd())
    {
        if (strncmp(arr.data(), "fmt ", 4) == 0)
        {
            arr = file_->read(sizeof(wave_info_) - 4);
            arr.insert(0, "fmt ");
            memcpy(&wave_info_, arr.data(), sizeof(wave_info_));

            do {
                arr = file_->read(1);
            } while (strncmp(arr.data(), "\0", 1) == 0);

            arr = file_->read(3);
            arr = file_->read(sizeof(wave_info_file_data_) - 4);
            arr.insert(0, "data");
            memcpy(&wave_info_file_data_, arr.data(), sizeof(wave_info_file_data_));

            if (strncmp(wave_info_file_data_.subchunk2Id, "data", 4) == 0)
                break;
        }
        arr = file_->read(4);
    }
}


//-----------------------------------------------------------------------------
// Получение фрагмента CUE *.WAVE формата
//-----------------------------------------------------------------------------
void ASound::getCUE_(QByteArray &baseStr)
{
    QByteArray cueChunckID("cue ");
    // Находим заголовок фрагмента cue
    int cueFirstByte = baseStr.indexOf(cueChunckID);
    // Если заголовок был найден
    if (cueFirstByte != -1)
    {
        // Загружаем во временный массив "шапку" фрагмента cue
        QByteArray tmp_data = baseStr.mid(cueFirstByte,
                                          sizeof(wave_cue_head_t));
        // Загружаем данные в структуру
        memcpy(&cue_head_, tmp_data.data(), sizeof(wave_cue_head_t));
        // Создаем временную структуру данных фрагмента cue
        wave_cue_data_t cue_data_t_;
        // Вычисляем смещение к первому блоку данных фрагмента cue
        int cue_data_offset = cueFirstByte + static_cast<int>(sizeof(wave_cue_head_t));
        // В цикле загружаем все данные точек cue
        for (int i = 1; i <= static_cast<int>(cue_head_.cueChunckPNum); ++i)
        {
            // Во временный массив - блок данных cue
            tmp_data = baseStr.mid(cue_data_offset,
                                   sizeof (wave_cue_data_t));
            // Данные во временную структуру
            memcpy(&cue_data_t_, tmp_data.data(),
                   sizeof(wave_cue_data_t));
            // Временную структуру в общий список cue-точек
            cue_data_.append(cue_data_t_);
            // Смещение к следующей точку cue
            cue_data_offset += static_cast<int>(sizeof(wave_cue_data_t));
        }

        canCUE_ = true;
    }
}



//-----------------------------------------------------------------------------
// Получение меток из фрагмента LIST->labls *.WAVE формата
//-----------------------------------------------------------------------------
void ASound::getLabels_(QByteArray &baseStr)
{
    // Читаем шапку блока LIST
    readWaveListChunckHeader_(baseStr);

    // Если был найден список
    if (strncasecmp(list_head_.chunckId, "list", 4) == 0)
    {
        QByteArray lablChunckId("labl"), tmp_data;

        int labelOffset = 0, labelFirstByte = 0;

        // Крутим пока не достигнем последней метки labl
        do
        {
            labelFirstByte = baseStr.indexOf(lablChunckId, labelOffset);
            int labelLength = 0; ///< Длина блока данных метки
            int labelCueID = 0; ///< ID связанной точки cue
            std::string labelName; ///< Имя метки

            if (labelFirstByte != -1)
            {
                // Парсим секцию labl вручную, так как не знаем заранее его длину. . .
                tmp_data = baseStr.mid(labelFirstByte + 4, 4);
                labelLength = tmp_data.at(0); // Получаем длину метки в байтах
                tmp_data = baseStr.mid(labelFirstByte + 8, 4);
                labelCueID = tmp_data.at(0); // Получаем ID точки cue
                tmp_data = baseStr.mid(labelFirstByte + 12, labelLength - 5);
                labelName = tmp_data.data(); // Получаем имя метки

                int index = 0; // Индекс для связанной точки cue в списке точек cue

                for (int k = 0; k < cue_data_.count(); ++k)
                    if (cue_data_[k].ID == labelCueID)
                    {
                        index = k;
                        break;
                    }

                wave_labels_.insert(QString::fromStdString(labelName),
                                    cue_data_[index].sampleOffset * static_cast<uint64_t>(wave_info_.bytesPerSample));

                canLABL_ = true;
            }

            labelOffset = labelFirstByte + 4; // Сдвигаем поиск на следующую метку
        } while (labelFirstByte != -1);
    }
}



//-----------------------------------------------------------------------------
// Чтение шапки фрагмента LIST файла wav
//-----------------------------------------------------------------------------
void ASound::readWaveListChunckHeader_(QByteArray &baseStr)
{
    QByteArray listChunckID("LIST");
    // Некоторые программы сохраняют фрагмент LIST - маленькими буквами
    QByteArray listChunckIDLower("list");
    // Номер первого байта списка
    int listFirstByte = baseStr.indexOf(listChunckID);
    listFirstByte =
            (listFirstByte == -1 ?
                 baseStr.indexOf(listChunckIDLower) : listFirstByte);
    if (listFirstByte != -1)
    {
        // Загружаем во временный массив блок "шапки" фрагмента LIST
        QByteArray tmp_data = baseStr.mid(listFirstByte,
                                          sizeof(wave_list_head_t));

        // Данные со временного массива - в структуру
        memcpy(&list_head_, tmp_data.data(),
               sizeof(wave_list_head_t));
    }
}



//-----------------------------------------------------------------------------
// Определение формата аудио (mono8/16 - stereo8/16)
//-----------------------------------------------------------------------------
void ASound::defineFormat_()
{
    if (canDo_)
    {
        if (wave_info_.bitsPerSample == 8)      // Если бит в сэмпле 8
        {
            if (wave_info_.numChannels == 1)    // Если 1 канал
            {
                format_ = AL_FORMAT_MONO8;
            }
            else                                // Если 2 канала
            {
                format_ = AL_FORMAT_STEREO8;
            }
        }
        else if (wave_info_.bitsPerSample == 16)// Если бит в сэмпле 16
        {
            if (wave_info_.numChannels == 1)    // Если 1 канал
            {
                format_ = AL_FORMAT_MONO16;
            }
            else                                // Если 2 канала
            {
                format_ = AL_FORMAT_STEREO16;
            }
        }
        else                                    // Если все плохо
        {
            setLastError("UNKNOWN_AUDIO_FORMAT");
            lastError_ = "UNKNOWN_AUDIO_FORMAT";
            canDo_ = false;
        }
    }
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void ASound::calcDuration_()
{
    if (canDo_)
    {
        for (int i = 0; i < BUFFER_BLOCKS; ++i)
        {
            double size = static_cast<double>(blockSize_[i]);
            double byteRate = static_cast<double>(wave_info_.byteRate);
            blockDuration_[i] = static_cast<uint64_t>(1000.0 * size / byteRate);
        }
    }
}



//-----------------------------------------------------------------------------
// Генерация буфера и источника
//-----------------------------------------------------------------------------
void ASound::generateStuff_()
{
    if (canDo_)
    {
        // Генерируем буфер
        alGenBuffers(BUFFER_BLOCKS, buffer_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_GENERATE_BUFFER";
            return;
        }

        // Генерируем источник
        alGenSources(1, &source_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_GENERATE_SOURCE";
            return;
        }

        // Настраиваем буфер
        for (int i = 0; i < BUFFER_BLOCKS; ++i)
        {
            alBufferData(buffer_[i], format_, wavData_[i], static_cast<ALsizei>(blockSize_[i]),
                         static_cast<ALsizei>(wave_info_.sampleRate));
        }

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_MAKE_BUFFER_DATA";
            return;
        }
    }
}



//-----------------------------------------------------------------------------
// Настройка источника
//-----------------------------------------------------------------------------
void ASound::configureSource_()
{
    if (canDo_)
    {
        if (canLABL_ && (blockDuration_[1] > 1))
        {
            // Если в файле предусмотрены метки цикла,
            // передаём источнику в очередь буфер начала,
            // несколько буферов зацикленной части (чтобы проигрывалась секунду)
            // и буфер конца
            ALuint buf[1];
            buf[0] = {buffer_[0]};
            alSourceQueueBuffers(source_, 1, buf);

            buf[0] = buffer_[1];
            num_cycle_blocks_ = 2 + 1000 / blockDuration_[1];
            for (uint64_t i = 0; i < num_cycle_blocks_; ++i)
                alSourceQueueBuffers(source_, 1, buf);

            buf[0] = buffer_[2];
            alSourceQueueBuffers(source_, 1, buf);

            emit notify(QString("| - Num cycle blocks: %1 (%2) ms").arg(num_cycle_blocks_).arg(num_cycle_blocks_ * blockDuration_[1]).toStdString());
        }
        else
        {
            // Передаём источнику первый буфер
            ALuint buf[1] = {buffer_[0]};
            alSourceQueueBuffers(source_, 1, buf);
        }

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_ADD_BUFFER_TO_SOURCE";
            return;
        }

        // Устанавливаем громкость
        alSourcef(source_, AL_GAIN, sourceVolume_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_VOLUME";
            return;
        }

        // Устанавливаем скорость воспроизведения
        alSourcef(source_, AL_PITCH, sourcePitch_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_PITCH";
            return;
        }

        // Устанавливаем зацикливание
        alSourcei(source_, AL_LOOPING, static_cast<char>(sourceLoop_));

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_LOOPING";
            return;
        }

        // Устанавливаем положение
        alSourcefv(source_, AL_POSITION, sourcePosition_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_POSITION";
            return;
        }

        // Устанавливаем вектор "скорости передвижения"
        alSourcefv(source_, AL_VELOCITY, sourceVelocity_);

        if (alGetError() != AL_NO_ERROR)
        {
            canDo_ = false;
            lastError_ = "CANT_APPLY_VELOCITY";
            return;
        }
    }
}



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
int ASound::getDuration(int idx)
{
    if (canDo_)
    {
        if (idx < 0)
        {
            int duration = 0;
            for (int i = 0; i < BUFFER_BLOCKS; ++i)
            {
                duration += blockDuration_[i];
            }
            return duration;
        }
        if (idx < BUFFER_BLOCKS)
            return blockDuration_[idx];
        return 0;
    }
    return 0;
}



//-----------------------------------------------------------------------------
// (слот) Установить громкость
//-----------------------------------------------------------------------------
void ASound::setVolume(float volume)
{
    if (canPlay_)
    {
        sourceVolume_ = volume;

        if (sourceVolume_ > MAX_SRC_VOLUME)
            sourceVolume_ = MAX_SRC_VOLUME;

        if (sourceVolume_ < MIN_SRC_VOLUME)
            sourceVolume_ = MIN_SRC_VOLUME;

        alSourcef(source_, AL_GAIN, sourceVolume_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть громкость
//-----------------------------------------------------------------------------
float ASound::getVolume()
{
    return sourceVolume_;
}



//-----------------------------------------------------------------------------
// (слот) Установить скорость воспроизведения
//-----------------------------------------------------------------------------
void ASound::setPitch(float pitch)
{
    if (canPlay_)
    {
        sourcePitch_ = pitch;
        alSourcef(source_, AL_PITCH, sourcePitch_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть скорость воспроизведения
//-----------------------------------------------------------------------------
float ASound::getPitch()
{
    return sourcePitch_;
}



//-----------------------------------------------------------------------------
// (слот) Установить зацикливание
//-----------------------------------------------------------------------------
void ASound::setLoop(bool loop)
{
    if (canPlay_)
    {
        if (canLABL_)
        {
            // Звук с метками зациклен другим механизмом
            sourceLoop_ = false;
        }
        else
        {
            sourceLoop_ = loop;
        }
        alSourcei(source_, AL_LOOPING, static_cast<char>(sourceLoop_));
    }
}



//-----------------------------------------------------------------------------
// Вернуть зацикливание
//-----------------------------------------------------------------------------
bool ASound::getLoop()
{
    return sourceLoop_;
}



//-----------------------------------------------------------------------------
// (слот) Установить положение
//-----------------------------------------------------------------------------
void ASound::setPosition(float x, float y, float z)
{
    if (canPlay_)
    {
        sourcePosition_[0] = x;
        sourcePosition_[1] = y;
        sourcePosition_[2] = z;
        alSourcefv(source_, AL_POSITION, sourcePosition_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть положение
//-----------------------------------------------------------------------------
void ASound::getPosition(float &x, float &y, float &z)
{
    x = sourcePosition_[0];
    y = sourcePosition_[1];
    z = sourcePosition_[2];
}



//-----------------------------------------------------------------------------
// (слот) Установить вектор "скорости передвижения"
//-----------------------------------------------------------------------------
void ASound::setVelocity(float x, float y, float z)
{
    if (canPlay_)
    {
        sourceVelocity_[0] = x;
        sourceVelocity_[1] = y;
        sourceVelocity_[2] = z;
        alSourcefv(source_, AL_VELOCITY, sourceVelocity_);
    }
}



//-----------------------------------------------------------------------------
// Вернуть вектор "скорости перемещения"
//-----------------------------------------------------------------------------
void ASound::getVelocity(float &x, float &y, float &z)
{
    x = sourceVelocity_[0];
    y = sourceVelocity_[1];
    z = sourceVelocity_[2];
}



//-----------------------------------------------------------------------------
// (слот) Играть звук
//-----------------------------------------------------------------------------
void ASound::play()
{
    if (!isPlaying())
    {
        if (canPlay_)
        {
            if (canLABL_)
            {
                // Таймер управления циклом
                if (timerControl_ == Q_NULLPTR)
                {
                    timerControl_ = new QTimer(this);
                    connect(timerControl_, SIGNAL(timeout()),
                            this, SLOT(onTimerLoopControl()));

                    // Таймер продолжительностью в половину зацикленной части
                    timerControl_->setInterval(blockDuration_[1] / 2);
                    timerControl_->setTimerType(Qt::PreciseTimer);
                }
                timerControl_->start();
//                emit notify(QString("START:duration:%1 ms").arg(blockDuration_[1] / 2, 6).toStdString());
            }

            alSourcePlay(source_);
        }
    }
    else
    {
        // Играем заново
        alSourcePlay(source_);
    }
}



//-----------------------------------------------------------------------------
// (слот) Приостановить звук
//-----------------------------------------------------------------------------
void ASound::pause()
{
    if (canPlay_)
    {
        alSourcePause(source_);
    }
}



//-----------------------------------------------------------------------------
// (слот) Остановить звук
//-----------------------------------------------------------------------------
void ASound::stop()
{
    if (canPlay_)
    {
        // Если у файла есть метки
        if (canLABL_)
        {
            // Задаём смещение на последний блок звука - завершающая часть
            alSourcei(source_, AL_BYTE_OFFSET,
                      static_cast<ALint>(blockSize_[0] + num_cycle_blocks_ * blockSize_[1]));

            // Таймер контроля цикла больше не нужен
            if (timerControl_ != Q_NULLPTR)
                if (timerControl_->isActive())
                    timerControl_->stop();
//            emit notify(QString("STOP:offset:%1").arg(blockSize_[0] + blockSize_[1] + blockSize_[1], 8).toStdString());
        }
        else
        {
            alSourceStop(source_);
        }
    }
}



//-----------------------------------------------------------------------------
// Вернуть последнюю ошибку
//-----------------------------------------------------------------------------
QString ASound::getLastError()
{
    QString foo = lastError_;
    lastError_.clear();
    return foo;
}



//-----------------------------------------------------------------------------
// Играет ли звук
//-----------------------------------------------------------------------------
bool ASound::isPlaying()
{
    ALint state;
    alGetSourcei(source_, AL_SOURCE_STATE, &state);
    return(state == AL_PLAYING);
}



//-----------------------------------------------------------------------------
// Приостановлен ли звук
//-----------------------------------------------------------------------------
bool ASound::isPaused()
{
    ALint state;
    alGetSourcei(source_, AL_SOURCE_STATE, &state);
    return(state == AL_PAUSED);
}



//-----------------------------------------------------------------------------
// Остановлен ли звук
//-----------------------------------------------------------------------------
bool ASound::isStopped()
{
    ALint state;
    alGetSourcei(source_, AL_SOURCE_STATE, &state);
    return(state == AL_STOPPED);
}



//-----------------------------------------------------------------------------
// Метод проверки необходимых параметров
//-----------------------------------------------------------------------------
void ASound::checkValue(std::string baseStr, const char targStr[], QString err)
{
    if (canDo_)
    {
        // // /////////////////////////////////////////////// //
        // // Важно, чтобы подстрока начиналась с 0 элемента! //
        // //    иначе проверку нельзя считать достоверной    //
        // // /////////////////////////////////////////////// //
        if (baseStr.find(targStr) != 0)
        {
            setLastError(err.toStdString());
            lastError_ = err;
            canDo_ = false;
        }
    }
}



//-----------------------------------------------------------------------------
// Работа с циклом
//-----------------------------------------------------------------------------
void ASound::onTimerLoopControl()
{
    // Текущий момент проигрываемого звука
    ALint curPosByte;
    alGetSourcei(source_, AL_BYTE_OFFSET, &curPosByte);
/*
    emit notify(QString("cur:%1|[0]%2|[0+1]%3|[0+num]%4|")
                    .arg(curPosByte, 8)
                    .arg(blockSize_[0], 8)
                    .arg(blockSize_[0] + blockSize_[1], 8)
                    .arg(blockSize_[0] + num_cycle_blocks_ * blockSize_[1], 8)
                    .toStdString());
*/
    ALint newPosByte = curPosByte;
    while (newPosByte >= static_cast<ALint>(blockSize_[0] + blockSize_[1]))
    {
        // Если находимся дальше второго буфера (в копиях зацикленной части),
        // смещаемся на тот же момент второго буфера (первой копии зацикленной части)
        newPosByte = newPosByte - static_cast<ALint>(blockSize_[1]);
    }
    if (newPosByte == curPosByte)
        return;

    alSourcei(source_, AL_BYTE_OFFSET, newPosByte);
/*
    emit notify(QString("offset: from%1|to%2|")
                    .arg(curPosByte, 8)
                    .arg(newPosByte, 8)
                    .toStdString());
*/
}



//-----------------------------------------------------------------------------
//
//      Класс управления очередью запуска звуков
//      (c) РГУПС, ВЖД 17/08/2017
//      Разработал: Ковшиков С. В.
//
//-----------------------------------------------------------------------------
/*!
 *  \file
 *  \brief Класс управления очередью запуска звуков
 *  \copyright РУГПС, ВЖД
 *  \author Ковшиков С. В.
 *  \date 17/08/2017
 */


//-----------------------------------------------------------------------------
// Конструктор
//-----------------------------------------------------------------------------
ASoundController::ASoundController(QObject *parent)
    : QObject(parent)
    , prepared_(false)
    , beginning_(false)
    , running_(false)
    , currentSoundIndex_(0)
    , soundPitch_(1.0f)
    , soundVolume_(100)
    , soundBegin_(Q_NULLPTR)
    , soundEnd_(Q_NULLPTR)
    , timerSoundChanger_(Q_NULLPTR)
{
    timerSoundChanger_ = new QTimer(this);
    timerSoundChanger_->setSingleShot(true);
    connect(timerSoundChanger_, SIGNAL(timeout()),
            this, SLOT(onTimerSoundChanger()));
}



//-----------------------------------------------------------------------------
// Деструктор
//-----------------------------------------------------------------------------
ASoundController::~ASoundController()
{

}



//-----------------------------------------------------------------------------
// Установить звук запуска
//-----------------------------------------------------------------------------
void ASoundController::setSoundBegin(QString soundPath)
{
    prepared_ = false;

    ASound* buf = new ASound(soundPath, this);

    if (buf->getLastError().isEmpty())
    {
        if (soundBegin_)
            delete soundBegin_;
        soundBegin_ = buf;
        soundBegin_->setVolume(soundVolume_);
    }
    else
    {
        delete buf;
    }

    prepare_();
}



//-----------------------------------------------------------------------------
// Добавить звук процесса работы
//-----------------------------------------------------------------------------
void ASoundController::appendSoundRunning(QString soundPath)
{
    prepared_ = false;

    ASound* buf = new ASound(soundPath, this);

    if (buf->getLastError().isEmpty())
    {
        listRunningSounds_.append(buf);
        listRunningSounds_.last()->setLoop(true);
    }
    else
    {
        delete buf;
    }

    prepare_();
}



//-----------------------------------------------------------------------------
// Установить список звуков процесса работы
//-----------------------------------------------------------------------------
void ASoundController::setSoundsRunning(QStringList soundPaths)
{
    prepared_ = false;

    clearRunningSoundsList_();

    for (QString path : soundPaths)
    {
        ASound* buf = new ASound(path, this);

        if (buf->getLastError().isEmpty())
        {
            listRunningSounds_.append(buf);
            listRunningSounds_.last()->setLoop(true);
        }
        else
        {
            delete buf;
        }
    }

    prepare_();
}



//-----------------------------------------------------------------------------
// Установить звук остановки
//-----------------------------------------------------------------------------
void ASoundController::setSoundEnd(QString soundPath)
{
    prepared_ = false;

    ASound* buf = new ASound(soundPath, this);

    if (buf->getLastError().isEmpty())
    {
        if (soundEnd_)
            delete soundEnd_;
        soundEnd_ = buf;
        soundEnd_->setVolume(soundVolume_);
    }

    prepare_();
}



//-----------------------------------------------------------------------------
// Запустить алгоритм воспроизведения (запуск устройства)
//-----------------------------------------------------------------------------
void ASoundController::begin()
{
    if (prepared_ && !running_ && !beginning_)
    {
        beginning_ = true;

        timerSoundChanger_->start();
        soundBegin_->play();
    }
}



//-----------------------------------------------------------------------------
// Установить звук процесса работы
//-----------------------------------------------------------------------------
void ASoundController::switchRunningSound(int index)
{
    if (running_)
    {
        if (index < listRunningSounds_.count() && index != currentSoundIndex_)
        {
            ASound* buf = listRunningSounds_[index];
            buf->setPitch(soundPitch_);
            buf->setVolume(soundVolume_);
            buf->play();
            listRunningSounds_[currentSoundIndex_]->stop();
            currentSoundIndex_ = index;
        }
    }
}



//-----------------------------------------------------------------------------
// Завершить алгоритм воспроизведения (остановка устройства)
//-----------------------------------------------------------------------------
void ASoundController::end()
{
    if (running_ || beginning_)
    {
        timerSoundChanger_->stop();
        soundEnd_->play();
        soundBegin_->stop();
        listRunningSounds_[currentSoundIndex_]->stop();
        beginning_ = false;
        running_ = false;
    }
}



//-----------------------------------------------------------------------------
// Установить скорость воспроизведения
//-----------------------------------------------------------------------------
void ASoundController::setPitch(float pitch)
{
    soundPitch_ = pitch;

    if (running_)
    {
        listRunningSounds_[currentSoundIndex_]->setPitch(pitch);
    }
}



//-----------------------------------------------------------------------------
// Установить громкость 0 - 100
//-----------------------------------------------------------------------------
void ASoundController::setVolume(int volume)
{
    soundVolume_ = volume;

    if (soundBegin_)
    {
        soundBegin_->setVolume(volume);
    }

    if (soundEnd_)
    {
        soundEnd_->setVolume(volume);
    }

    if (running_)
    {
        listRunningSounds_[currentSoundIndex_]->setVolume(volume);
    }
}



//-----------------------------------------------------------------------------
// Аварийно завершить алгоритм вопсроизведения в любой момент
//-----------------------------------------------------------------------------
void ASoundController::forcedStop()
{
    timerSoundChanger_->stop();
    soundBegin_->stop();
    running_ = false;
    for (ASound* sound : listRunningSounds_)
    {
        sound->stop();
    }
}



//-----------------------------------------------------------------------------
// Слот обработки таймера переключения звуков
//-----------------------------------------------------------------------------
void ASoundController::onTimerSoundChanger()
{
    currentSoundIndex_ = 0;
    ASound* buf = listRunningSounds_[currentSoundIndex_];
    buf->setPitch(soundPitch_);
    buf->setVolume(soundVolume_);
    buf->play();
    beginning_ = false;
    running_ = true;
}



//-----------------------------------------------------------------------------
// Проверить готовность всех звуков
//-----------------------------------------------------------------------------
void ASoundController::prepare_()
{
    if ( (soundBegin_ != Q_NULLPTR) &&
         (soundEnd_ != Q_NULLPTR) &&
         (listRunningSounds_.count() > 0) )
    {
        timerSoundChanger_->setInterval(soundBegin_->getDuration());
        prepared_ = true;
    }
}



//-----------------------------------------------------------------------------
// Очистить список фаз процесса работы
//-----------------------------------------------------------------------------
void ASoundController::clearRunningSoundsList_()
{
    if (listRunningSounds_.isEmpty())
        return;

    for (ASound* sound : listRunningSounds_)
    {
        delete sound;
    }

    listRunningSounds_.clear();
}
