#include "AnimatedDatabasePager.h"

#include "AnimatedPagedLOD.h"
#include "FindCustomAnimationsVisitor.h"
#include "FindModelAnimations.h"
#include "Logger.h"

#include <vsg/io/ReaderWriter.h>
#include <vsg/io/read.h>
#include <vsg/nodes/Transform.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/CullGroup.h>
#include <vsg/threading/atomics.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/utils/ComputeBounds.h>
#include <vsg/utils/SharedObjects.h>
#include <vsg/utils/PropagateDynamicObjects.h>

#include <GPUMemoryMonitor.h>
#include <ResourceCollector.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimatedDatabasePager::start(uint32_t numReadThreads)
{
    //LOG_INFO("AnimatedDatabasePager::start(%u)", numReadThreads);

    auto readThread = [this](AnimatedDatabasePager& animatedDatabasePager, const std::string& threadName)
    {
        LOG_INFO("Started %s", threadName.c_str());

        //auto local_instrumentation = shareOrDuplicateForThreadSafety(animatedDatabasePager.instrumentation);
        //if (local_instrumentation) local_instrumentation->setThreadName(threadName);

        while (animatedDatabasePager.status->active())
        {
            vsg::ref_ptr<vsg::PagedLOD> plod = animatedDatabasePager._requestQueue->take_when_available(animatedDatabasePager.frameCount.load());
            if (plod)
            {
                //CPU_INSTRUMENTATION_L1_NC(animatedDatabasePager.instrumentation, "AnimatedDatabasePager read", COLOR_PAGER);

                // ============ ДОБАВИТЬ ПРОВЕРКУ ============
                // Если узел помечен на удаление - пропускаем
                if (plod->requestStatus.load() == vsg::PagedLOD::DeleteRequest)
                {
                    animatedDatabasePager.requestDiscarded(plod.get());
                    continue;
                }
                // ============================================

                uint64_t frameDelta = animatedDatabasePager.frameCount - plod->frameHighResLastUsed.load();

                if (frameDelta > 1 || !vsg::compare_exchange(plod->requestStatus, vsg::PagedLOD::ReadRequest, vsg::PagedLOD::Reading))
                {
                    LOG_WARN("AnimatedDatabasePager: Expired (%u/%u) read request for model from file: %s",
                             plod->frameHighResLastUsed.load(), animatedDatabasePager.frameCount.load(), plod->filename.string().c_str());
                    animatedDatabasePager.requestDiscarded(plod);
                    continue;
                }

                ++(plod->loadAttempts);

                vsg::ref_ptr<vsg::Node> node = plod->pending;
                if (!node)
                {
                    vsg::ref_ptr<vsg::Object> loaded = animatedDatabasePager.dedupRead(plod->filename, plod->options);
                    node = loaded.cast<vsg::Node>();

                    if (!node)
                    {
                        LOG_WARN("AnimatedDatabasePager: fail to load model from file: %s", plod->filename.string().c_str());

                        auto error = loaded.cast<vsg::ReadError>();
                        if (error)
                        {
                            LOG_WARN(error->message.c_str());
                        }

                        animatedDatabasePager.requestDiscarded(plod);
                        continue;
                    }

                    // Считаем, что инициализируем все PagedLOD с нулевым угловым размером отсечения
                    if (plod->children[0].minimumScreenHeightRatio <= 0.0)
                    {
                        // При первой загрузке настраиваем ограничивающую сферу для отсечения
                        if (auto cullnode = node.cast<vsg::CullNode>())
                        {
                            // Пробуем использовать готовую сферу из автосоздаваемого CullNode
                            plod->bound = cullnode->bound;
                            node = cullnode->child;
                        }
                        else if (auto cullgroup = node.cast<vsg::CullGroup>())
                        {
                            // Пробуем использовать готовую сферу из автосоздаваемого CullGroup
                            plod->bound = cullgroup->bound;
                            node = vsg::Group::create(cullgroup->children.begin(), cullgroup->children.end());
                        }
                        else if (auto bounds = vsg::visit<vsg::ComputeBounds>(node).bounds)
                        {
                            // Рассчитываем ограничивающую сферу вручную утилитой ComputeBounds
                            const vsg::dvec3 center = (bounds.max + bounds.min) * 0.5;
                            const double radius = vsg::length(bounds.max - bounds.min) * 0.5;
                            plod->bound = vsg::dsphere(center, radius);
                        }
                        else
                        {
                            plod->bound = vsg::dsphere(0.0, 0.0, 0.0, 1.0);
                        }

                        // Вручную задаём ненулевой радиус объектам
                        // с нулевой ограничивающей сферой (например,
                        // модели с источником света, но без меша)
                        if (plod->bound.radius <= 0.0)
                        {
                            plod->bound.radius = 0.5;
                        }
                        else
                        {
                            plod->bound.radius += 0.5;
                        }

                        // Настраиваем угловой размер отсечения
                        plod->children[0].minimumScreenHeightRatio = animatedDatabasePager.cullingScreenHeightRatio;
                    }
                    else
                    {
                        // Если угловой размер отсечения не нулевой,
                        // мы уже загружали модель и настраивали ограничивающую сферу,
                        // просто убираем лишние CullNode
                        if (auto cullnode = node.cast<vsg::CullNode>())
                        {
                            node = cullnode->child;
                        }
                        else if (auto cullgroup = node.cast<vsg::CullGroup>())
                        {
                            node = vsg::Group::create(cullgroup->children.begin(), cullgroup->children.end());
                        }
                    }

                    // Создаём, конфигурируем анимации и подключаем их к сигналам симулятора
                    if (auto aplod = plod.cast<AnimatedPagedLOD>())
                    {
                        node = animatedDatabasePager.loadAnimations(aplod, node);
                    }
                }

                if (compare_exchange(plod->requestStatus, vsg::PagedLOD::Reading, vsg::PagedLOD::Compiling))
                {
                    {
                        std::scoped_lock<std::mutex> lock(animatedDatabasePager.pendingPagedLODMutex);
                        plod->pending = node;
                    }

                    try
                    {
                        // compile plod
                        if (auto result = animatedDatabasePager.compileManager->compile(node))
                        {
                            plod->requestStatus.exchange(vsg::PagedLOD::MergeRequest);

                            // move to the merge queue;
                            animatedDatabasePager._toMergeQueue->add(plod, result);

                            //LOG_INFO("AnimatedDatabasePager: load and compiled model from file: %s", plod->filename.string().c_str());
                        }
                        else
                        {
                            LOG_WARN("AnimatedDatabasePager: fail to compile model from file: %s", plod->filename.string().c_str());                            
                            animatedDatabasePager.requestDiscarded(plod);
                        }
                    }
                    catch (const vsg::Exception& e)
                    {
                        animatedDatabasePager.requestDiscarded(plod);
                        LOG_WARN("AnimatedDatabasePager: vsg::Exception (%s) while compiling model from file: %s", e.message.c_str(), plod->filename.string().c_str());
                    }
                    catch (const std::exception& e)
                    {
                        animatedDatabasePager.requestDiscarded(plod);
                        LOG_WARN("AnimatedDatabasePager: std::exception (%s) while compiling model from file: %s", e.what(), plod->filename.string().c_str());
                    }
                    catch (...)
                    {
                        animatedDatabasePager.requestDiscarded(plod);
                        LOG_WARN("AnimatedDatabasePager: exception while compiling model from file: %s", plod->filename.string().c_str());
                    }
                }
                else
                {
                    animatedDatabasePager.requestDiscarded(plod);
                }
            }
            else
            {
                // sleep for a frame.
                std::this_thread::sleep_for(std::chrono::milliseconds(32));
                continue;
            }
        }
        //LOG_INFO("Finished %s", threadName.c_str());
    };

    auto deleteThread = [](const AnimatedDatabasePager& animatedDatabasePager, const std::string& threadName)
    {
        LOG_INFO("Started %s", threadName.c_str());

        while (animatedDatabasePager.status->active())
        {
            animatedDatabasePager.deleteQueue->wait_then_clear();
        }
        //LOG_INFO("Finished %s", threadName.c_str());
    };

    for (uint32_t i = 0; i < numReadThreads; ++i)
    {
        threads.emplace_back(readThread, std::ref(*this), vsg::make_string("AnimatedDatabasePager read thread ", i));
    }

    threads.emplace_back(deleteThread, std::ref(*this), "AnimatedDatabasePager delete thread");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> AnimatedDatabasePager::loadAnimations(vsg::ref_ptr<AnimatedPagedLOD> aplod,
                                           vsg::ref_ptr<vsg::Node> node)
{
    aplod->animations_map->thread_safe_clear();

    if (aplod->animations_dir.empty())
    {
        return node;
    }

    {
        // Model's animations
        FindModelAnimationsCreateInfo fma_create_info = {node, aplod->animations_map, aplod->animations_dir};
        auto find_model_animations = FindModelAnimations::create(fma_create_info);
    }

    // Custom animations for model
    auto pdo = vsg::PropagateDynamicObjects::create();

    vsg::CopyOp copyop;
    auto duplicate = copyop.duplicate = new vsg::Duplicate;

    FindCustomAnimationsVisitorCreateInfo fcav_create_info = {pdo, duplicate, aplod->animations_dir, aplod->animations_map};

    FindCustomAnimationsVisitor fcav(fcav_create_info);
    node->accept(fcav);

    node->traverse(*pdo);

    // Copy all animated parts of shared model for independent behaviour
    if (!pdo->dynamicObjects.empty())
    {
        for (auto& object : pdo->dynamicObjects)
        {
            if (!duplicate->contains(object))
            {
                duplicate->insert(object);
            }
        }

        duplicate->insert(node);
        node = copyop(node);
        fcav.reconfigure_animations();
    }
    return node;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Object> AnimatedDatabasePager::dedupRead(
    const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options)
{
    const auto key = filename.string();

    std::shared_future<vsg::ref_ptr<vsg::Object>> future;
    bool iAmLoader = false;
    std::shared_ptr<std::promise<vsg::ref_ptr<vsg::Object>>> promise;

    {
        std::scoped_lock lock(_readMutex);
        auto it = _pendingReads.find(key);
        if (it != _pendingReads.end())
        {
            future = it->second;
            ++_dedupWaits;
        }
        else
        {
            promise = std::make_shared<std::promise<vsg::ref_ptr<vsg::Object>>>();
            future = promise->get_future().share();
            _pendingReads.emplace(key, future);
            iAmLoader = true;
            ++_actualLoads;
        }
        ++_totalReads;
    }

    if (iAmLoader)
    {
        vsg::ref_ptr<vsg::Object> result;
        try
        {
            result = vsg::read(filename, options);
        }
        catch (...)
        {
            promise->set_exception(std::current_exception());
            std::scoped_lock lock(_readMutex);
            _pendingReads.erase(key);
            return {};
        }
        promise->set_value(result);

        std::scoped_lock lock(_readMutex);
        _pendingReads.erase(key);

        return result;
    }
    else
    {
        LOG_INFO("dedupRead: reusing in-flight load for %s", key.c_str());
        try
        {
            return future.get();
        }
        catch (...)
        {
            LOG_WARN("dedupRead: in-flight load failed for %s", key.c_str());
            return {};
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimatedDatabasePager::reportDedupStats() const
{
    LOG_INFO("dedupRead stats: %u total reads, %u actual loads, %u deduplicated waits",
             _totalReads.load(), _actualLoads.load(), _dedupWaits.load());
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimatedDatabasePager::updateSceneGraph(vsg::ref_ptr<vsg::FrameStamp> frameStamp,
                                             vsg::CompileResult &cr)
{
    vsg::DatabasePager::updateSceneGraph(frameStamp, cr);

    // Периодическая проверка памяти
    //if (frameStamp && (frameStamp->frameCount % 60 == 0))
    //{
        checkAndUnloadIfMemoryLimitExceeded();
    //}
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimatedDatabasePager::setGPUMemoryMonitor(vsg::ref_ptr<GPUMemoryMonitor> memory_monitor)
{
    _memory_monitor = memory_monitor;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimatedDatabasePager::checkAndUnloadIfMemoryLimitExceeded()
{
    if (_memory_monitor == nullptr)
    {
        return;
    }

    _memory_monitor->update();

    if (_memory_monitor->isThresholdExceeded())
    {
        LOG_WARN("GPU memory threshold reached: %f", _memory_monitor->getCurrentUsagePercent());

        forceUnloadInvisibleObjects();

        _memory_monitor->update();

        if (_memory_monitor->isThresholdExceeded())
        {
            uint32_t newTarget = std::max(this->targetMaxNumPagedLODWithHighResSubgraphs / 2, 1u);
            this->targetMaxNumPagedLODWithHighResSubgraphs = newTarget;
            LOG_INFO("Reduced targetMaxNumPagedLODWithHighResSubgraphs to %u", newTarget);
        }
    }
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<vsg::ref_ptr<vsg::PagedLOD>> AnimatedDatabasePager::collectInvisibleObjects()
{
    std::vector<vsg::ref_ptr<vsg::PagedLOD>> invisibleObjects;

    if (!pagedLODContainer)
    {
        return invisibleObjects;
    }

    std::scoped_lock lock(pendingPagedLODMutex);

    auto &elements = pagedLODContainer->elements;

    uint32_t index = pagedLODContainer->inactiveList.head;

    while (index != 0)
    {
        auto &element = elements[index];

        if (element.plod)
        {
            invisibleObjects.push_back(element.plod);
        }

        index = element.next;
    }

    std::sort(invisibleObjects.begin(), invisibleObjects.end(),
              [](const vsg::ref_ptr<vsg::PagedLOD>& a, const vsg::ref_ptr<vsg::PagedLOD>& b) {
                  return a->frameHighResLastUsed.load() < b->frameHighResLastUsed.load();
              });

    return invisibleObjects;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool AnimatedDatabasePager::unloadPagedLOD(vsg::PagedLOD *plod)
{
    if (plod == nullptr) return false;

    // 1. Проверяем, не используется ли узел в текущем кадре
    uint64_t currentFrame = frameCount.load();
    uint64_t lastUsed = plod->frameHighResLastUsed.load();
    if (currentFrame - lastUsed < 2)
    {
        // Узел использовался в прошлом или текущем кадре — не удаляем
        LOG_DEBUG("Skipping unload: node used recently, frame=%llu, lastUsed=%llu",
                  currentFrame, lastUsed);
        return false;
    }

    vsg::PagedLOD::RequestStatus expected = vsg::PagedLOD::NoRequest;
    if (!vsg::compare_exchange(plod->requestStatus, expected, vsg::PagedLOD::DeleteRequest))
    {
        return false;
    }

    LOG_INFO("Unloading invisible object: %s", plod->filename.string().c_str());

    // 2. Удаляем узел из culledPagedLODs, чтобы RecordTraversal не использовал его
    if (culledPagedLODs)
    {
        auto& highresCulled = culledPagedLODs->highresCulled;
        highresCulled.erase(
            std::remove(highresCulled.begin(), highresCulled.end(), plod),
            highresCulled.end()
            );

        auto& newHighresRequired = culledPagedLODs->newHighresRequired;
        newHighresRequired.erase(
            std::remove(newHighresRequired.begin(), newHighresRequired.end(), plod),
            newHighresRequired.end()
            );
    }

    // 3. Собираем и освобождаем ресурсы
    /*ResourceCollector collector;
    if (plod->children[0].node)
    {
        plod->children[0].node->accept(collector);
    }
    if (plod->pending)
    {
        plod->pending->accept(collector);
    }
    collector.releaseAll();*/

    // 4. Очищаем ссылки внутри мьютекса
    {
        std::scoped_lock lock(pendingPagedLODMutex);

        if (plod->children[0].node)
        {
            plod->children[0].node = nullptr;
        }

        if (plod->pending)
        {
            if (deleteQueue)
            {
                std::list<vsg::ref_ptr<vsg::Object>> deleteList;
                deleteList.push_back(plod->pending);
                deleteQueue->add(deleteList);
            }
            plod->pending = nullptr;
        }
    }

    // 5. Удаляем из контейнера PagedLOD
    if (pagedLODContainer && plod->index != 0)
    {
        pagedLODContainer->remove(plod);
    }

    // 6. Сбрасываем состояние
    plod->requestStatus.exchange(vsg::PagedLOD::NoRequest);
    plod->requestCount.exchange(0);

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
uint32_t AnimatedDatabasePager::forceUnloadInvisibleObjects()
{
    if (!pagedLODContainer)
    {
        return 0;
    }

    uint64_t currentFrame = frameCount.load();
    auto invisibleObjects = collectInvisibleObjects();

    uint32_t unloadedCount = 0;

    for (auto &plod : invisibleObjects)
    {
        if (unloadPagedLOD(plod))
        {
            unloadedCount++;
        }
    }

    if (unloadedCount > 0)
    {
        LOG_INFO("Unloaded %u invisible objects", unloadedCount);
    }

    return unloadedCount;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimatedDatabasePager::requestDiscarded(vsg::PagedLOD* plod)
{
    // Если узел помечен на удаление, просто сбрасываем состояние
    if (plod->requestStatus.load() == vsg::PagedLOD::DeleteRequest)
    {
        plod->requestStatus.exchange(vsg::PagedLOD::NoRequest);
        plod->requestCount.exchange(0);
        return;
    }

    vsg::DatabasePager::requestDiscarded(plod);
}
