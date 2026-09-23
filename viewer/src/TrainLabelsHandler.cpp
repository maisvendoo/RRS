#include "TrainLabelsHandler.h"

#include "Logger.h"
#include "filesystem.h"
#include "settings.h"

#include <vsg/io/Options.h>
#include <vsg/io/read.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/CullNode.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Switch.h>
#include <vsg/text/Font.h>
#include <vsg/text/StandardLayout.h>
#include <vsg/text/Text.h>

#include <QString>

#include <algorithm>

namespace
{
    /// Радиус сферы отсечения подписи поезда относительно размера шрифта
    constexpr double TRAIN_LABEL_CULLING_COEFF = 5.0;

    /// Наименование поезда для отображения в подписи (при пустом имени - "Поезд #N")
    QString labelDisplayName(const simulator_train_update_t& train, int train_id)
    {
        return train.train_name.isEmpty()
            ? QString("Поезд #%1").arg(train_id)
            : train.train_name;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TrainLabelsHandler::TrainLabelsHandler(const settings_t& settings)
    : labels_text_font_size(static_cast<double>(settings.train_labels_text_font_size))
    , labels_text_shift(settings.train_labels_text_shift)
    , labels_text_scale_distance(settings.train_labels_text_scale_distance)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainLabelsHandler::setup(vsg::ref_ptr<vsg::Options> options,
                               const std::vector<simulator_train_update_t>& trains)
{
    // Очищаем предыдущий граф сцены
    labels.clear();
    root = nullptr;

    FileSystem& fs = FileSystem::getInstance();
    const std::string font_path = fs.combinePath(fs.getFontsDir(), "JetBrainsMono-Regular.ttf");

    auto font = vsg::read_cast<vsg::Font>(font_path, options);
    if (!font)
    {
        LOG_ERROR("Fail to load train name labels font: %s", font_path.c_str());
        return false;
    }

    auto group = vsg::Group::create();

    const float text_height = static_cast<float>(labels_text_font_size);

    int train_id = 0;
    for (const auto& train : trains)
    {
        Label label;

        // Наименование поезда; при пустом имени выводим "Поезд #N"
        const QString name = labelDisplayName(train, train_id);
        label.display_name = name.toStdString();

        auto layout = vsg::StandardLayout::create();
        layout->horizontalAlignment = vsg::StandardLayout::CENTER_ALIGNMENT;
        layout->position = vsg::vec3(0.0f, 0.0f, 0.0f);
        layout->horizontal = vsg::vec3(text_height, 0.0f, 0.0f);
        layout->vertical = vsg::vec3(0.0f, text_height, 0.0f);
        //layout->outlineWidth = 0.5f;
        layout->billboard = true;
        layout->billboardAutoScaleDistance = static_cast<float>(labels_text_scale_distance);

        auto text = vsg::Text::create();
        text->text = vsg::wstringValue::create(name.toStdWString());
        text->font = font;
        text->layout = layout;
        text->setup(0, options);

        label.transform = vsg::MatrixTransform::create();
        label.transform->addChild(text);

        label.cull_node = vsg::CullNode::create();
        label.cull_node->bound = vsg::dsphere(0.0, 0.0, 0.0,
                                              labels_text_font_size * TRAIN_LABEL_CULLING_COEFF);
        label.cull_node->child = label.transform;

        group->addChild(label.cull_node);

        labels.emplace_back(std::move(label));
        ++train_id;
    }

    root = vsg::Switch::create();
    root->addChild(vsg::MASK_ALL, group);

    LOG_INFO("Created scene graph for %d train name labels",
             static_cast<int>(labels.size()));

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainLabelsHandler::step(const std::vector<VehicleExterior>& vehicles,
                              const std::vector<simulator_train_update_t>& trains)
{
    if (labels.empty() || !root)
        return;

    const std::size_t count = std::min(labels.size(), trains.size());

    for (std::size_t i = 0; i < count; ++i)
    {
        // Позиция над первым вагоном поезда
        const int first_vehicle_id = trains[i].first_vehicle_id;
        if (first_vehicle_id < 0 ||
            static_cast<std::size_t>(first_vehicle_id) >= vehicles.size())
        {
            continue;
        }

        const vsg::dvec3 position = vehicles[first_vehicle_id].position + labels_text_shift;

        labels[i].transform->matrix = vsg::translate(position);
        labels[i].cull_node->bound.center = position;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainLabelsHandler::needRebuild(const std::vector<simulator_train_update_t>& trains) const
{
    if (labels.size() != trains.size())
        return true;

    for (std::size_t i = 0; i < labels.size(); ++i)
    {
        if (labels[i].display_name != labelDisplayName(trains[i], static_cast<int>(i)).toStdString())
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
vsg::ref_ptr<vsg::Node> TrainLabelsHandler::getRootNode() const
{
    return root;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TrainLabelsHandler::setVisible(bool visible)
{
    if (root)
        root->setAllChildren(visible);
}
