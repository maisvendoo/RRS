//------------------------------------------------------------------------------
//
//      Менеджер автосохранений сессии (ТЗ "RP-сервер", п.4-8):
//      - XML + CRC32, атомарная запись через QSaveFile
//      - ротация 10 файлов, откат к предыдущему валидному при порче
//      - автозагрузка последнего валидного сейва (AutoLoad)
//      - сохранение в фоновом потоке (низкий приоритет)
//
//------------------------------------------------------------------------------

#ifndef SESSION_SAVE_MANAGER_H
#define SESSION_SAVE_MANAGER_H

#include    <QObject>
#include    <QString>
#include    <QStringList>

#include    "session-types.h"

#include    <atomic>
#include    <condition_variable>
#include    <mutex>
#include    <thread>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SessionSaveManager : public QObject
{
    Q_OBJECT

public:

    explicit SessionSaveManager(QObject *parent = nullptr);
    ~SessionSaveManager() override;

    /// Прочитать session.xml, запустить рабочий поток
    void init(const QString &cfg_path, const QString &base_dir);

    bool isEnabled() const;

    /// Интервал автосохранения, с (по умолчанию 600)
    double saveInterval() const;

    bool isAutoloadEnabled() const;

    /// Список файлов сейвов (полные пути), свежие первыми
    QStringList saveFiles() const;

    /// Последний валидный сейв (проверка CRC32), "" если нет
    QString latestValidSave(QString *error = nullptr) const;

    /// Загрузить сейв с проверкой CRC32
    bool loadSession(const QString &path,
                     session::session_state_t &state,
                     QString *error = nullptr) const;

    /// Асинхронно сохранить снимок (фоновый поток, очередь глубины 1)
    void saveAsync(const session::session_state_t &state);

private:

    /// Сериализация в XML + строка CRC32
    QByteArray serialize(const session::session_state_t &state) const;
    bool deserialize(const QByteArray &data,
                     session::session_state_t &state,
                     QString *error) const;

    bool writeSave(const session::session_state_t &state,
                   QString *error);

    void rotate() const;

    static QString savePath(int index);

    bool enabled = true;
    bool autoload = true;
    double save_interval = 600.0;

    QString base_dir = "";

    std::thread worker;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    session::session_state_t pending_state;
    bool has_pending = false;
    std::atomic<bool> stop_flag{false};
};

#endif // SESSION_SAVE_MANAGER_H
