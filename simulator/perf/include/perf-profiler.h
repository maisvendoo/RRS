//------------------------------------------------------------------------------
//
//      Perf profiler (встроенный профилировщик систем)
//      ТЗ "Оптимизация", п.1, 23
//
//      Измеряемое время выполнения каждой системы: обёртка scope-таймер
//      аккумулирует длительности, счётчики кадра, top-N дорогих систем.
//      Статистика ведётся скользящим окном (N кадров), поэтому FPS и
//      время кадра отражают текущее состояние, а не среднее за сессию.
//      Поезда шагают в своих потоках - Profiler потокобезопасен (mutex).
//
//------------------------------------------------------------------------------

#ifndef     PERF_PROFILER_H
#define     PERF_PROFILER_H

#include    <QString>

#include    <chrono>
#include <cstdint>
#include <mutex>
#include <vector>

namespace perf
{

class Profiler;

/// Таймер области: замер от создания до разрушения
class ScopedTimer
{
public:

    ScopedTimer(Profiler& profiler, const char* system_name);
    ~ScopedTimer();

private:

    Profiler& profiler_;
    const char* name_;
    std::chrono::steady_clock::time_point start_;
};

/// Система в статистике
struct SystemStat
{
    QString name = "";
    double total_ms = 0.0;      ///< Суммарно за окно
    double max_ms = 0.0;        ///< Худший кадр
    std::uint64_t calls = 0;    ///< Число вызовов
    double budget_ms = 0.0;     ///< Бюджет на кадр (0 - нет)
};

//------------------------------------------------------------------------------
/// Профилировщик
//------------------------------------------------------------------------------
class Profiler
{
public:

    /// Начать кадр (сбрасывает почасовые аккумуляторы по finalize)
    void beginFrame();

    /// Зафиксировать замер (вызывается ScopedTimer)
    void record(const char* name, double ms);

    /// Завершить кадр
    void endFrame();

    /// Среднее время кадра, мс
    double getFrameMs() const;

    /// FPS
    double getFPS() const;

    /// Статистика систем за окно (отсортировано по среднему времени
    /// окна, топ-запрос)
    std::vector<SystemStat> getTopSystems(std::size_t top_n) const;

    /// Задать бюджет системе, мс/кадр (п.22)
    void setBudget(const QString& system, double ms);

    /// Система превышает бюджет (для adaptive performance)
    bool isOverBudget(const QString& system) const;

    /// Сброс окна статистики (вызывается автоматически из endFrame
    /// каждые window_size кадров)
    void resetWindow();

private:

    friend class ScopedTimer;

    struct Accumulator
    {
        QString name;
        double frame_ms = 0.0;      ///< Время в текущем кадре, мс
        double window_ms = 0.0;     ///< Суммарно за окно, мс
        double max_ms = 0.0;        ///< Худший замер в окне, мс
        std::uint64_t calls = 0;    ///< Число вызовов за окно
        double budget_ms = 0.0;
    };

    /// Аккумулятор по имени (создаётся при первом обращении).
    /// Вызывать при удержанном mutex_
    Accumulator& acc(const QString& name);

    /// Сброс окна без блокировки (вызывается при удержанном mutex_)
    void resetWindowNoLock();

    std::vector<Accumulator> systems_;

    /// Всего кадров с начала сессии
    std::uint64_t frames_ = 0;
    /// Суммарное время всех кадров сессии, мс
    double total_ms_ = 0.0;

    /// Кадров в текущем окне статистики
    std::uint64_t window_frames_ = 0;
    /// Суммарное время кадров текущего окна, мс
    double window_ms_total_ = 0.0;

    /// Размер скользящего окна, кадров
    static constexpr std::uint64_t window_size_ = 120;

    std::chrono::steady_clock::time_point frame_start_;

    /// Поезда шагают в своих потоках - весь доступ под блокировкой
    mutable std::mutex mutex_;
};

} // namespace perf

#endif // PERF_PROFILER_H
