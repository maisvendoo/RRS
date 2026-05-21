#include    <GPUMemoryMonitor.h>
#include    <vsg/vk/MemoryBufferPools.h>
#include    <Logger.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GPUMemoryMonitor::init(vsg::ref_ptr<vsg::MemoryBufferPools> memoryPools)
{
    _memoryPools = memoryPools;
    update();

    if (_memoryPools)
    {
        LOG_INFO("GPUMemoryMonitor initialized: total reserved = %llu MB",
                 (unsigned long long)(_currentUsageBytes / (1024 * 1024)));
    }
    else
    {
        LOG_WARN("GPUMemoryMonitor initialized without memory pools");
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GPUMemoryMonitor::setMemoryLimitMB(uint64_t limitMB)
{
    _memoryLimitBytes = limitMB << 20;
    LOG_INFO("GPU memory limit set to %llu MB", static_cast<unsigned long long>(limitMB));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GPUMemoryMonitor::setThreshold(float threshold)
{
    _threshold = std::clamp(threshold, 0.5f, 0.95f);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GPUMemoryMonitor::update()
{
    updateUsage();
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint64_t GPUMemoryMonitor::getCurrentUsageMB() const
{
    return _currentUsageBytes / 1024 / 1024;
}

uint64_t GPUMemoryMonitor::getMemoryLimitMB() const
{
    return _memoryLimitBytes;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
float GPUMemoryMonitor::getCurrentUsagePercent() const
{
    if (_memoryLimitBytes == 0)
    {
        return 0.0f;
    }

    return 100.0f * _currentUsageBytes / _memoryLimitBytes;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool GPUMemoryMonitor::isThresholdExceeded() const
{
    if (_memoryLimitBytes == 0)
    {
        return false;
    }

    return _currentUsageBytes > static_cast<uint64_t>(_memoryLimitBytes * _threshold);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void GPUMemoryMonitor::updateUsage()
{
    if (_memoryPools)
    {
        _currentUsageBytes = _memoryPools->computeMemoryTotalReserved();
        _availableBytes = _memoryPools->computeMemoryTotalAvailable();
    }
}
