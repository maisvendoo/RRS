#ifndef ANIMATED_DATABASE_PAGER_H
#define ANIMATED_DATABASE_PAGER_H

#include <vsg/io/DatabasePager.h>
#include <atomic>
#include <future>
#include <mutex>
#include <unordered_map>
#include <chrono>

struct animations_t;
class AnimatedPagedLOD;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AnimatedDatabasePager : public vsg::Inherit<vsg::DatabasePager, AnimatedDatabasePager>
{
public:
    AnimatedDatabasePager() = default;

    double cullingScreenHeightRatio = 0.005;

    void start(uint32_t numReadThreads = 4) override;

    vsg::ref_ptr<vsg::Object> dedupRead(const vsg::Path& filename,
                                        vsg::ref_ptr<const vsg::Options> options);

    void reportDedupStats() const;

    // ========= УПРАВЛЕНИЕ ПАМЯТЬЮ =========

    /// Установить лимит видеопамяти в мегабайтах (0 = без лимита)
    void setGPUMemoryLimitMB(uint64_t limit_mb) {
        gpuMemoryLimitBytes = limit_mb * 1024 * 1024;
    }

    /// Установить порог срабатывания очистки (0.0 - 1.0)
    void setCleanupThreshold(float threshold) {
        cleanupThreshold = std::clamp(threshold, 0.5f, 0.95f);
    }

    /// Принудительно выгрузить все невидимые объекты
    uint32_t forceUnloadInvisibleObjects();

    /// Выгрузить невидимые объекты, если память превышает порог
    uint32_t unloadInvisibleIfNeeded();

    /// Очистить кэш дедупликации
    void clearDedupCache();

protected:
    ~AnimatedDatabasePager() = default;

    void requestDiscarded(vsg::PagedLOD* plod);

private:
    vsg::ref_ptr<vsg::Node> loadAnimations(vsg::ref_ptr<AnimatedPagedLOD> aplod,
                                           vsg::ref_ptr<vsg::Node> node);

    // ========= ПОЛЯ ДЛЯ УПРАВЛЕНИЯ ПАМЯТЬЮ =========
    uint64_t gpuMemoryLimitBytes = 2ull * 1024 * 1024 * 1024;  // 2 GB по умолчанию
    float cleanupThreshold = 0.85f;  // 85%

    /// Собрать невидимые объекты
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> collectInvisibleObjects(uint64_t currentFrame);

    /// Выгрузить конкретный PagedLOD
    bool unloadPagedLOD(vsg::PagedLOD* plod);

    // ========= СУЩЕСТВУЮЩИЕ ПОЛЯ =========
    std::mutex _readMutex;
    std::unordered_map<std::string, std::shared_future<vsg::ref_ptr<vsg::Object>>> _pendingReads;

    std::atomic<uint32_t> _totalReads{0};
    std::atomic<uint32_t> _actualLoads{0};
    std::atomic<uint32_t> _dedupWaits{0};
};

#endif // ANIMATED_DATABASE_PAGER_H
