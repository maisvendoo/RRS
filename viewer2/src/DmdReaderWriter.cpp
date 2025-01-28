#include "DmdReaderWriter.h"
#include "Logger.h"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/core/Array.h>
#include <vsg/core/Data.h>
#include <vsg/core/External.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/io/read.h>
#include <vsg/maths/vec3.h>
#include <vsg/nodes/Geometry.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/nodes/VertexIndexDraw.h>
#include <vsg/raytracing/TopLevelAccelerationStructure.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/state/Descriptor.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/state/DescriptorSetLayout.h>
#include <vsg/state/GraphicsPipeline.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/Sampler.h>
#include <vsg/state/ShaderModule.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>
#include <vsg/utils/GraphicsPipelineConfigurator.h>
#include <vsg/utils/ShaderSet.h>
#include <vulkan/vulkan_core.h>

DmdReaderWriter::DmdReaderWriter()
{

}

DmdReaderWriter::~DmdReaderWriter()
{

}

vsg::ref_ptr<vsg::Object> DmdReaderWriter::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    auto extension = filename.substr(filename.find_last_of('.'));
    if (extension != ".dmdu")
    {
        return {};
    }

    std::ifstream united_file(filename);
    if (!united_file)
    {
        LOG_ERROR("United file %s does not open", filename.c_str());
        return {};
    }

    LOG_INFO("%s", filename.c_str());

    std::string model_path;
    std::string texture_path;
    std::getline(united_file, model_path);
    std::getline(united_file, texture_path);
    united_file.close();

    std::ifstream model_file(model_path);
    if (!model_file)
    {
        LOG_ERROR("Model file %s does not open", model_path.c_str());
        return {};
    }

    std::string buffer {};
    while (buffer != "TriMesh()")
    {
        model_file >> buffer;
        if (model_file.eof())
        {
            LOG_ERROR("No TriMesh() in %s", filename.c_str());
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t position_count;
    std::uint32_t position_face_count;
    model_file >> position_count >> position_face_count;

    std::uint32_t index_count = position_face_count * 3;

    model_file >> buffer >> buffer;

    std::vector<vsg::vec3> positions(position_count);
    for (auto& position : positions)
    {
        for (int i = 0; i < 3; ++i)
        {
            model_file >> position[i];
            if (model_file.fail() || !std::isfinite(position[i]))
            {
                LOG_ERROR("Wrong position value in %s", filename.c_str());
            }
        }
    }

    model_file >> buffer >> buffer >> buffer >> buffer;

    std::vector<std::uint32_t> position_indices(index_count);
    for (auto& index : position_indices)
    {
        model_file >> index;
        --index;
    }

    while (buffer != "Texture:")
    {
        model_file >> buffer;
        if (model_file.fail() || model_file.eof())
        {
            LOG_ERROR("Failed to find \"Texture:\" in %s", filename.c_str());
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t tex_coord_count;
    std::uint32_t tex_coord_face_count;
    model_file >> tex_coord_count >> tex_coord_face_count;

    if (position_face_count != tex_coord_face_count)
    {
        LOG_ERROR("Different position and tex coord face count in %s", filename.c_str());
    }

    model_file >> buffer >> buffer;

    std::vector<vsg::vec2> tex_coords(tex_coord_count);
    for (auto& tex_coord : tex_coords)
    {
        for (int i { 0 }; i < 2; ++i)
        {
            model_file >> tex_coord[i];
            if (model_file.fail() || !std::isfinite(tex_coord[i]))
            {
                LOG_ERROR("Wrong tex coord value in %s", filename.c_str());
            }
        }

        model_file >> buffer;
    }

    model_file >> buffer >> buffer >> buffer >> buffer >> buffer;

    std::vector<std::uint32_t> tex_coord_indices(index_count);
    for (auto& index : tex_coord_indices)
    {
        model_file >> index;
        --index;
    }

    std::vector<vsg::vec3> vertices;
    std::vector<vsg::vec2> final_tex_coords;
    std::vector<std::uint32_t> indices;

    vertices.reserve(index_count);
    final_tex_coords.reserve(index_count);
    indices.resize(index_count);

    std::map<std::pair<int, int>, int> unique_indices {};
    for (int i = 0; i < index_count; ++i)
    {
        const std::uint32_t position_index = position_indices[i];
        const std::uint32_t tex_coord_index = tex_coord_indices[i];

        const auto found_it { unique_indices.find(std::make_pair(position_index, tex_coord_index)) };
        if (found_it == unique_indices.end())
        {
            vertices.emplace_back(positions[position_index]);
            final_tex_coords.emplace_back(tex_coords[tex_coord_index]);
            indices[i] = vertices.size() - 1;
            unique_indices.emplace(std::make_pair(position_index, tex_coord_index), vertices.size() - 1);
        }
        else
        {
            indices[i] = found_it->second;
        }
    }

    vertices.shrink_to_fit();
    final_tex_coords.shrink_to_fit();

    if (vertices.empty() || final_tex_coords.empty() || indices.empty())
    {
        LOG_ERROR("vertices/texCoords/indices are empty in %s", filename.c_str());
    }

    auto vsg_vertices = vsg::vec3Array::create(vertices.size(), vertices.data());
    auto vsg_tex_coords = vsg::vec2Array::create(final_tex_coords.size(), final_tex_coords.data());
    auto vsg_indices = vsg::uintArray::create(indices.size(), indices.data());

    LOG_INFO("vsg_vertex count: %u", vertices.size());
    LOG_INFO("vsg_tex_coords count: %u", vertices.size());
    LOG_INFO("vsg_indices count: %u", vertices.size());

    auto texture_data = vsg::read_cast<vsg::Data>(texture_path, options);
    if (!texture_data)
    {
        LOG_ERROR("Failed to load texture %s", texture_path.c_str());
        return {};
    }

    // vsg::ref_ptr<vsg::ShaderStage> vertex_shader_stage = vsg::ShaderStage::read(VK_SHADER_STAGE_VERTEX_BIT, "main", "viewer2/vert.spv", options);
    // if (!vertex_shader_stage)
    // {
    //     LOG_ERROR("Failed to load vertex shader");
    //     return {};
    // }

    // vsg::ref_ptr<vsg::ShaderStage> fragment_shader_stage = vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", "viewer2/frag.spv", options);
    // if (!fragment_shader_stage)
    // {
    //     LOG_ERROR("Failed to load fragment shader");
    //     return {};
    // }

    vsg::Paths searchPaths = vsg::getEnvPaths("VSG_FILE_PATH");
    vsg::ref_ptr<vsg::ShaderStage> vertexShader = vsg::ShaderStage::read(VK_SHADER_STAGE_VERTEX_BIT, "main", vsg::findFile("shaders/vert_PushConstants.spv", searchPaths));
    vsg::ref_ptr<vsg::ShaderStage> fragmentShader = vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", vsg::findFile("shaders/frag_PushConstants.spv", searchPaths));

    //------------------------------------------------------------------------------------------------------------------

    // set up graphics pipeline
    vsg::DescriptorSetLayoutBindings descriptorBindings{
        {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorType, descriptorCount, stageFlags, pImmutableSamplers}
    };

    auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

    vsg::PushConstantRanges pushConstantRanges{
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection, view, and model matrices, actual push constant calls automatically provided by the VSG's RecordTraversal
    };

    auto colors = vsg::vec3Array::create(vsg_vertices->size());
    for (int i = 0; i < colors->size(); ++i)
    {
        colors->at(i).set(1.0f, 1.0f, 1.0f);
    }

    vsg::VertexInputState::Bindings vertexBindingsDescriptions{
        VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // vertex data
        VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // colour data
        VkVertexInputBindingDescription{2, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}  // tex coord data
    };

    vsg::VertexInputState::Attributes vertexAttributeDescriptions{
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}, // colour data
        VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0}     // tex coord data
    };

    vsg::GraphicsPipelineStates pipelineStates{
        vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
        vsg::InputAssemblyState::create(),
        vsg::RasterizationState::create(),
        vsg::MultisampleState::create(),
        vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create()};

    auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

    // create texture image and associated DescriptorSets and binding
    auto texture = vsg::DescriptorImage::create(vsg::Sampler::create(), texture_data, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{texture});
    auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSet);

    // create StateGroup as the root of the scene/command graph to hold the GraphicsPipeline, and binding of Descriptors to decorate the whole graph
    auto scenegraph = vsg::StateGroup::create();
    scenegraph->add(bindGraphicsPipeline);
    scenegraph->add(bindDescriptorSet);

    // set up model transformation node
    auto transform = vsg::MatrixTransform::create(); // VK_SHADER_STAGE_VERTEX_BIT

    // add transform to root of the scene graph
    scenegraph->addChild(transform);

    auto drawCommands = vsg::Commands::create();
    drawCommands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{vsg_vertices, colors, vsg_tex_coords}));
    drawCommands->addChild(vsg::BindIndexBuffer::create(vsg_indices));
    drawCommands->addChild(vsg::DrawIndexed::create(vsg_indices->size(), 1, 0, 0, 0));

    // add drawCommands to transform
    transform->addChild(drawCommands);

    return scenegraph;

    // auto graphicsPipelineConfig = vsg::GraphicsPipelineConfigurator::create(shader_set);
    // graphicsPipelineConfig->assignTexture("texSampler", texture_data);

    // // auto mat = vsg::PhongMaterialValue::create();
    // // mat->value().diffuse.set(1.0f, 1.0f, 1.0f, 1.0f);
    // // mat->value().specular.set(1.0f, 0.0f, 0.0f, 1.0f); // red specular highlight

    // // graphicsPipelineConfig->assignDescriptor("material", mat);

    // vsg::DataList vertexArrays;

    // graphicsPipelineConfig->assignArray(vertexArrays, "inPosition", VK_VERTEX_INPUT_RATE_VERTEX, vsg_vertices);
    // graphicsPipelineConfig->assignArray(vertexArrays, "inTexCoord", VK_VERTEX_INPUT_RATE_VERTEX, vsg_tex_coords);

    // auto drawCommands = vsg::Commands::create();
    // drawCommands->addChild(vsg::BindVertexBuffers::create(graphicsPipelineConfig->baseAttributeBinding, vertexArrays));
    // drawCommands->addChild(vsg::BindIndexBuffer::create(vsg_indices));
    // drawCommands->addChild(vsg::DrawIndexed::create(vsg_indices->size(), 1, 0, 0, 0));

    // graphicsPipelineConfig->init();

    // auto stateGroup = vsg::StateGroup::create();

    // graphicsPipelineConfig->copyTo(stateGroup);

    // auto transform = vsg::MatrixTransform::create();

    // stateGroup->addChild(drawCommands);
    // transform->addChild(stateGroup);

    // return transform;

    // auto descriptor_set_layout = vsg::DescriptorSetLayout::create(vsg::DescriptorSetLayoutBindings{
    //     VkDescriptorSetLayoutBinding{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}
    // });;

    // auto pipeline_layout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptor_set_layout}, vsg::PushConstantRanges{});

    // auto vertex_input_bindings = vsg::VertexInputState::Bindings{
    //     VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX},
    //     VkVertexInputBindingDescription{1, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}
    // };

    // auto vertex_input_attributes = vsg::VertexInputState::Attributes{
    //     VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    //     VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32_SFLOAT, 0}
    // };

    // auto graphics_pipeline = vsg::GraphicsPipeline::create(
    //     pipeline_layout,
    //     shader_stages,
    //     vsg::GraphicsPipelineStates{
    //         vsg::InputAssemblyState::create(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
    //         vsg::RasterizationState::create(),
    //         vsg::MultisampleState::create(),
    //         vsg::ColorBlendState::create(),
    //         vsg::DepthStencilState::create(),
    //         vsg::VertexInputState::create(vertex_input_bindings, vertex_input_attributes)
    //     }
    // );

    // auto geometry = vsg::Geometry::create();
    // geometry->assignArrays(vsg::DataList{vsg_vertices, vsg_tex_coords});
    // geometry->assignIndices(vsg_indices);
    // geometry->commands.push_back(vsg::DrawIndexed::create(vsg_indices->size(), 1, 0, 0, 0));

    // auto state_group = vsg::StateGroup::create();
    // state_group->add(vsg::BindGraphicsPipeline::create(graphics_pipeline));

    // auto sampler = vsg::Sampler::create();
    // auto texture_descriptor = vsg::DescriptorImage::create(sampler, texture_data, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    // auto descriptor_set = vsg::DescriptorSet::create(descriptor_set_layout, vsg::Descriptors{texture_descriptor});
    // state_group->add(vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, descriptor_set));
    // state_group->addChild(geometry);

    // LOG_INFO("Model loaded");
    // return state_group;
}
