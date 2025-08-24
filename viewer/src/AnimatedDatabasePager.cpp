#include "AnimatedDatabasePager.h"

#include "AnimatedPagedLOD.h"
#include "FindCustomAnimationsVisitor.h"
#include "FindModelAnimations.h"
#include "Logger.h"

#include <vsg/io/ReaderWriter.h>
#include <vsg/io/read.h>
#include <vsg/nodes/Transform.h>
#include <vsg/nodes/Cullnode.h>
#include <vsg/threading/atomics.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/utils/SharedObjects.h>
#include <vsg/utils/PropagateDynamicObjects.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimatedDatabasePager::start(uint32_t numReadThreads)
{
    auto readThread = [](vsg::ref_ptr<vsg::DatabaseQueue> requestQueue,
                         vsg::ref_ptr<vsg::ActivityStatus> status,
                         AnimatedDatabasePager& animatedDatabasePager)
    {
        while (status->active())
        {
            vsg::ref_ptr<vsg::PagedLOD> plod = requestQueue->take_when_available();
            if (plod)
            {
                uint64_t frameDelta = animatedDatabasePager.frameCount - plod->frameHighResLastUsed.load();
                if (frameDelta > 1 || !vsg::compare_exchange(plod->requestStatus, vsg::PagedLOD::ReadRequest, vsg::PagedLOD::Reading))
                {
                    animatedDatabasePager.requestDiscarded(plod);
                    continue;
                }

                vsg::ref_ptr<vsg::Object> loaded = vsg::read(plod->filename, plod->options);

                auto node = loaded.cast<vsg::Node>();
                if (!node)
                {
                    LOG_WARN("AnimatedDatabasePager: fail to load model from file: %s", plod->filename.c_str());

                    auto error = loaded.cast<vsg::ReadError>();
                    if (error)
                    {
                        LOG_WARN(error->message.c_str());
                    }

                    animatedDatabasePager.requestDiscarded(plod);
                    continue;
                }

                if (compare_exchange(plod->requestStatus, vsg::PagedLOD::Reading, vsg::PagedLOD::Compiling))
                {
                    if (auto cullnode = node.cast<vsg::CullNode>())
                    {
                        node = cullnode->child;
                    }
                    if (auto transform = node.cast<vsg::Transform>())
                    {
                        transform->subgraphRequiresLocalFrustum = false;
                    }

                    if (auto aplod = plod.cast<AnimatedPagedLOD>())
                    {
                        animatedDatabasePager.loadAnimations(aplod);
                    }

                    {
                        std::scoped_lock<std::mutex> lock(animatedDatabasePager.pendingPagedLODMutex);
                        plod->pending = node;
                    }

                    // compile plod
                    if (auto result = animatedDatabasePager.compileManager->compile(node))
                    {
                        plod->requestStatus.exchange(vsg::PagedLOD::MergeRequest);

                        // move to the merge queue;
                        animatedDatabasePager._toMergeQueue->add(plod, result);
                    }
                    else
                    {
                        LOG_WARN("Failed to compile model from file: %s", plod->filename.c_str());
                        animatedDatabasePager.requestDiscarded(plod);
                    }
                }
                else
                {
                    animatedDatabasePager.requestDiscarded(plod);
                }
            }
        }
    };

    auto deleteThread = [](vsg::ref_ptr<vsg::DeleteQueue> deleteQueue,
                           vsg::ref_ptr<vsg::ActivityStatus> status,
                           const AnimatedDatabasePager& databasePager)
    {
        while (status->active())
        {
            deleteQueue->wait_then_clear();
        }
    };

    for (uint32_t i = 0; i < numReadThreads; ++i)
    {
        threads.emplace_back(readThread, std::ref(_requestQueue), std::ref(_status), std::ref(*this));
    }

    threads.emplace_back(deleteThread, std::ref(_deleteQueue), std::ref(_status), std::ref(*this));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void AnimatedDatabasePager::loadAnimations(vsg::ref_ptr<AnimatedPagedLOD> node)
{
    node->animations->thread_safe_clear();

    if (node->animations_dir.empty())
        return;

    {
        // Model's animations
        FindModelAnimationsCreateInfo fma_create_info = {node, node->animations, node->animations_dir};
        auto find_model_animations = FindModelAnimations::create(fma_create_info);
    }
    std::size_t old_size = node->animations->animations.size();

    // Custom animations for model
    auto pdo = vsg::PropagateDynamicObjects::create();

    vsg::CopyOp copyop;
    auto duplicate = copyop.duplicate = new vsg::Duplicate;

    FindCustomAnimationsVisitorCreateInfo fcav_create_info = {pdo, duplicate, node->animations_dir, node->animations};

    FindCustomAnimationsVisitor fcav(fcav_create_info);
    node->accept(fcav);
    LOG_INFO("AnimatedDatabasePager: loaded %zu model and %zu custom (total: %zu) animations from %s for model %s",
             old_size,
             node->animations->animations.size() - old_size,
             node->animations->animations.size(),
             node->animations_dir.c_str(),
             node->filename.c_str());

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
}
