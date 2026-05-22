#ifndef     RESOURCE_COLLECTOR_H
#define     RESOURCE_COLLECTOR_H

#include    <vsg/core/ConstVisitor.h>
#include    <vsg/state/BufferInfo.h>
#include    <vsg/state/ImageInfo.h>
#include    <vsg/nodes/Geometry.h>
#include    <vsg/nodes/VertexDraw.h>
#include    <vsg/nodes/VertexIndexDraw.h>
#include    <vsg/commands/BindVertexBuffers.h>
#include    <vsg/commands/BindIndexBuffer.h>
#include    <vsg/state/DescriptorBuffer.h>
#include    <vsg/state/DescriptorImage.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ResourceCollector : public vsg::ConstVisitor
{
public:

    std::vector<vsg::ref_ptr<vsg::BufferInfo>> buffers;
    std::vector<vsg::ref_ptr<vsg::ImageView>> imageViews;
    std::vector<vsg::ref_ptr<vsg::Data>> datas;

    void clear()
    {
        buffers.clear();
        imageViews.clear();
        datas.clear();
    }

    size_t totalCount() const
    {
        return buffers.size() + imageViews.size() + datas.size();
    }

    void releaseAll()
    {
        for (auto &bi : buffers)
        {
            if (bi)
            {
                if (bi->buffer)
                {
                    bi->buffer = nullptr;
                }

                bi->data = nullptr;
            }
        }

        for (auto &iv : imageViews)
        {
            if (iv)
            {
                iv->image = nullptr;
                iv = nullptr;
            }
        }

        for (auto &data : datas)
        {
            data = nullptr;
        }
    }

    void apply(const vsg::Node &node) override
    {
        node.traverse(*this);
    }

    void apply(const vsg::Geometry &geometry) override
    {
        for (auto &bi : geometry.arrays)
        {
            if (bi)
            {
                buffers.push_back(bi);
            }
        }

        if (geometry.indices)
        {
            buffers.push_back(geometry.indices);
        }

        geometry.traverse(*this);
    }

    void apply(const vsg::VertexDraw& vd) override
    {
        for (auto& bi : vd.arrays)
            if (bi) buffers.push_back(bi);
        vd.traverse(*this);
    }

    void apply(const vsg::VertexIndexDraw& vid) override
    {
        for (auto& bi : vid.arrays)
            if (bi) buffers.push_back(bi);
        if (vid.indices) buffers.push_back(vid.indices);
        vid.traverse(*this);
    }

    void apply(const vsg::BindVertexBuffers& bvb) override
    {
        for (auto& bi : bvb.arrays)
            if (bi) buffers.push_back(bi);
        bvb.traverse(*this);
    }

    void apply(const vsg::BindIndexBuffer& bib) override
    {
        if (bib.indices) buffers.push_back(bib.indices);
        bib.traverse(*this);
    }

    void apply(const vsg::DescriptorBuffer& db) override
    {
        for (auto& bi : db.bufferInfoList)
            if (bi) buffers.push_back(bi);
        db.traverse(*this);
    }

    void apply(const vsg::DescriptorImage& di) override
    {
        for (auto& ii : di.imageInfoList)
        {
            if (ii && ii->imageView)
            {
                imageViews.push_back(ii->imageView);
            }
        }
        di.traverse(*this);
    }

    void apply(const vsg::ImageInfo& ii) override
    {
        if (ii.imageView)
        {
            imageViews.push_back(ii.imageView);
        }
    }

    void apply(const vsg::BufferInfo& bi) override
    {
        // Исправлено: создаём ref_ptr из указателя
        buffers.push_back(vsg::ref_ptr<vsg::BufferInfo>(const_cast<vsg::BufferInfo*>(&bi)));
    }

    void apply(const vsg::Data& data) override
    {
        // Исправлено: создаём ref_ptr из указателя
        datas.push_back(vsg::ref_ptr<vsg::Data>(const_cast<vsg::Data*>(&data)));
    }
};

#endif
