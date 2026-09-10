#pragma once
#ifndef TRAIN_LABELS_HANDLER_H
#define TRAIN_LABELS_HANDLER_H

#include <VehicleExterior.h>
#include <simulator-update-struct.h>

#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>

#include <string>
#include <vector>

struct settings_t;

namespace vsg
{
    class CullNode;
    class MatrixTransform;
    class Node;
    class Options;
    class Switch;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TrainLabelsHandler final
{
public:
    explicit TrainLabelsHandler(const settings_t& settings);
    ~TrainLabelsHandler() = default;

    /// Создание (пересоздание) графа сцены подписей поездов
    bool setup(vsg::ref_ptr<vsg::Options> options,
               const std::vector<simulator_train_update_t>& trains);

    /// Обновление позиций подписей над первым вагоном каждого поезда
    void step(const std::vector<VehicleExterior>& vehicles,
              const std::vector<simulator_train_update_t>& trains);

    /// Количество созданных подписей
    size_t getLabelCount() const noexcept { return labels.size(); }

    /// true, если список поездов (количество или наименования) изменился
    bool needRebuild(const std::vector<simulator_train_update_t>& trains) const;

    /// Корневой узел графа сцены подписей поездов
    vsg::ref_ptr<vsg::Node> getRootNode() const;

    /// Включение/выключение отображения подписей поездов
    void setVisible(bool visible);

private:
    struct Label
    {
        /// Текст подписи
        std::string display_name;

        /// Сфера отсечения (обновляется при движении поезда)
        vsg::ref_ptr<vsg::CullNode> cull_node;

        /// Позиция подписи в сцене
        vsg::ref_ptr<vsg::MatrixTransform> transform;
    };

    /// Размер шрифта подписей поездов, м
    double labels_text_font_size = 8.0;

    /// Смещение подписей поездов относительно позиции первого вагона, м
    vsg::dvec3 labels_text_shift = {0.0, 0.0, 12.0};

    /// Дистанция, после которой подписи уменьшаются с расстоянием, м
    double labels_text_scale_distance = 500.0;

    /// Список подписей поездов (индекс соответствует индексу поезда)
    std::vector<Label> labels;

    /// Корневой узел подписей (для включения/отключения отображения)
    vsg::ref_ptr<vsg::Switch> root;
};

#endif // TRAIN_LABELS_HANDLER_H