#ifndef     GPU_MEMORY_MONITOR_H
#define     GPU_MEMORY_MONITOR_H

#include    <vsg/vk/DeviceMemory.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class GPUMemoryMonitor : public vsg::Inherit<vsg::Object, GPUMemoryMonitor>
{
public:

    GPUMemoryMonitor() = default;

    void init(vsg::ref_ptr<vsg::MemoryBufferPools> memoryPools);

    void setMemoryLimitMB(uint64_t limitMB);

    void setThreshold(float threshold);

    void update();

    uint64_t getCurrentUsageMB() const;

    uint64_t getMemoryLimitMB() const;

    float getCurrentUsagePercent() const;

    bool isThresholdExceeded() const;


private:

    vsg::ref_ptr<vsg::MemoryBufferPools> _memoryPools;

    uint64_t _memoryLimitBytes = 0;

    float _threshold = 0.85f;

    uint64_t _currentUsageBytes = 0;

    uint64_t _availableBytes = 0;

    void updateUsage();
};

#endif
