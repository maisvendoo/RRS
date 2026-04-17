#ifndef ANIMATED_DATABASE_PAGER_H
#define ANIMATED_DATABASE_PAGER_H

#include <vsg/io/DatabasePager.h>

#include <atomic>
#include <future>
#include <mutex>
#include <unordered_map>

struct animations_t;
class AnimatedPagedLOD;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class AnimatedDatabasePager : public vsg::Inherit<vsg::DatabasePager, AnimatedDatabasePager>
{
public:
    AnimatedDatabasePager() = default;

    void start(uint32_t numReadThreads = 4) override;

    /// Thread-safe read that deduplicates concurrent loads of the same file.
    /// Only one thread loads a given filename; others wait for the result.
    vsg::ref_ptr<vsg::Object> dedupRead(const vsg::Path& filename,
                                        vsg::ref_ptr<const vsg::Options> options);

    void reportDedupStats() const;

protected:
    ~AnimatedDatabasePager() = default;

private:

    vsg::ref_ptr<vsg::Node> loadAnimations(vsg::ref_ptr<AnimatedPagedLOD> aplod,
                                           vsg::ref_ptr<vsg::Node> node);

    std::mutex _readMutex;
    std::unordered_map<std::string, std::shared_future<vsg::ref_ptr<vsg::Object>>> _pendingReads;

    std::atomic<uint32_t> _totalReads{0};
    std::atomic<uint32_t> _actualLoads{0};
    std::atomic<uint32_t> _dedupWaits{0};
};

#endif // ANIMATED_DATABASE_PAGER_H
