//------------------------------------------------------------------------------
//
//      Perf profiler
//
//------------------------------------------------------------------------------

#include    "perf-profiler.h"

#include    <algorithm>

namespace perf
{

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ScopedTimer::ScopedTimer(Profiler& profiler, const char* system_name)
    : profiler_(profiler)
    , name_(system_name)
    , start_(std::chrono::steady_clock::now())
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ScopedTimer::~ScopedTimer()
{
    const auto end = std::chrono::steady_clock::now();

    const std::chrono::duration<double, std::milli> d = end - start_;

    profiler_.record(name_, d.count());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Profiler::beginFrame()
{
    std::lock_guard<std::mutex> lock(mutex_);

    frame_start_ = std::chrono::steady_clock::now();

    for (Accumulator& a : systems_)
        a.frame_ms = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Profiler::record(const char* name, double ms)
{
    std::lock_guard<std::mutex> lock(mutex_);

    Accumulator& a = acc(QString(name));

    a.frame_ms += ms;
    a.window_ms += ms;
    a.max_ms = std::max(a.max_ms, ms);
    ++a.calls;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Profiler::endFrame()
{
    const auto end = std::chrono::steady_clock::now();

    const std::chrono::duration<double, std::milli> d = end - frame_start_;

    std::lock_guard<std::mutex> lock(mutex_);

    // Статистика сессии (вся история) и скользящего окна (текущее
    // состояние): окно пересоздаётся каждые window_size кадров
    total_ms_ += d.count();
    ++frames_;

    window_ms_total_ += d.count();
    ++window_frames_;

    if (window_frames_ >= window_size_)
        resetWindowNoLock();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Profiler::getFrameMs() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (window_frames_ == 0)
        return 0.0;

    return window_ms_total_ / static_cast<double>(window_frames_);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Profiler::getFPS() const
{
    const double ms = getFrameMs();

    if (ms <= 0.0)
        return 0.0;

    return 1000.0 / ms;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<SystemStat> Profiler::getTopSystems(std::size_t top_n) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    const double frames = static_cast<double>(
                std::max<std::uint64_t>(window_frames_, 1));

    std::vector<SystemStat> stats;
    stats.reserve(systems_.size());

    for (const Accumulator& a : systems_)
    {
        SystemStat stat;
        stat.name = a.name;
        stat.total_ms = a.window_ms;
        stat.max_ms = a.max_ms;
        stat.calls = a.calls;
        stat.budget_ms = a.budget_ms;

        stats.push_back(stat);
    }

    // Сортировка по убыванию среднего времени окна (мс на кадр)
    std::sort(stats.begin(), stats.end(),
              [frames](const SystemStat& lhs, const SystemStat& rhs)
    {
        return lhs.total_ms / frames > rhs.total_ms / frames;
    });

    if (stats.size() > top_n)
        stats.resize(top_n);

    return stats;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Profiler::setBudget(const QString& system, double ms)
{
    std::lock_guard<std::mutex> lock(mutex_);

    acc(system).budget_ms = std::max(ms, 0.0);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Profiler::isOverBudget(const QString& system) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (const Accumulator& a : systems_)
    {
        if (a.name != system)
            continue;

        // Без бюджета или без набранного окна сравнивать не с чем
        if (a.budget_ms <= 0.0 || window_frames_ == 0)
            return false;

        return (a.window_ms / static_cast<double>(window_frames_)) >
                a.budget_ms;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Profiler::resetWindow()
{
    std::lock_guard<std::mutex> lock(mutex_);

    resetWindowNoLock();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Profiler::resetWindowNoLock()
{
    window_ms_total_ = 0.0;
    window_frames_ = 0;

    for (Accumulator& a : systems_)
    {
        a.window_ms = 0.0;
        a.max_ms = 0.0;
        a.calls = 0;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Profiler::Accumulator& Profiler::acc(const QString& name)
{
    for (Accumulator& a : systems_)
    {
        if (a.name == name)
            return a;
    }

    Accumulator a;
    a.name = name;
    systems_.push_back(a);

    return systems_.back();
}

} // namespace perf
