#include "AnimatedDatabasePager.h"

#include "AnimatedPagedLOD.h"
#include "FindCustomAnimationsVisitor.h"
#include "FindModelAnimations.h"
#include "Logger.h"

#include <vsg/io/ReaderWriter.h>
#include <vsg/io/read.h>
#include <vsg/nodes/Transform.h>
#include <vsg/nodes/CullNode.h>
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

                vsg::ref_ptr<vsg::Node> node = loaded.cast<vsg::Node>();
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

                if (compare_exchange(plod->requestStatus, vsg::PagedLOD::Reading, vsg::PagedLOD::Compiling))
                {
                    if (auto cullnode = node.cast<vsg::CullNode>())
                    {
                        node = cullnode->child;
                    }
                    /*if (auto transform = node.cast<vsg::Transform>())
                    {
                        transform->subgraphRequiresLocalFrustum = false;
                    }*/

                    if (auto aplod = plod.cast<AnimatedPagedLOD>())
                    {
                        node = animatedDatabasePager.loadAnimations(aplod, node);
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

                        //LOG_INFO("AnimatedDatabasePager: load and compiled model from file: %s", plod->filename.string().c_str());
                    }
                    else
                    {
                        LOG_WARN("AnimatedDatabasePager: fail to compile model from file: %s", plod->filename.string().c_str());
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
        (void)databasePager;

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
vsg::ref_ptr<vsg::Node> AnimatedDatabasePager::loadAnimations(vsg::ref_ptr<AnimatedPagedLOD> aplod,
                                           vsg::ref_ptr<vsg::Node> node)
{
    aplod->animations_map->thread_safe_clear();

    if (aplod->animations_dir.empty())
        return node;

    {
        // Model's animations
        FindModelAnimationsCreateInfo fma_create_info = {node, aplod->animations_map, aplod->animations_dir};
        auto find_model_animations = FindModelAnimations::create(fma_create_info);
    }
    std::size_t old_size = aplod->animations_map->animations.size();

    // Custom animations for model
    auto pdo = vsg::PropagateDynamicObjects::create();

    vsg::CopyOp copyop;
    auto duplicate = copyop.duplicate = new vsg::Duplicate;

    FindCustomAnimationsVisitorCreateInfo fcav_create_info = {pdo, duplicate, aplod->animations_dir, aplod->animations_map};

    FindCustomAnimationsVisitor fcav(fcav_create_info);
    node->accept(fcav);

    LOG_INFO("AnimatedDatabasePager: loaded %zu model and %zu custom (total: %zu) animations from %s for model %s",
             old_size,
             aplod->animations_map->animations.size() - old_size,
             aplod->animations_map->animations.size(),
             aplod->animations_dir.c_str(),
             aplod->filename.string().c_str());

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
