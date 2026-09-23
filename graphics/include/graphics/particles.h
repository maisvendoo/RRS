#ifndef GRAPHICS_PARTICLES_H
#define GRAPHICS_PARTICLES_H

#include <vsg/core/Array.h>
#include <vsg/core/ref_ptr.h>
#include <vsg/maths/vec3.h>
#include <vsg/maths/vec4.h>

#include <cstddef>
#include <vector>

namespace vsg
{

class Camera;
class Node;

}

//------------------------------------------------------------------------------
// CPU-система частиц (ТЗ "Частицы": дым/пар из выхлопной трубы, брызги
// из-под колёс). Только пресеты High/Ultra/Extreme — Legacy/Low не создают
// её вовсе.
//
// Чистый VSG 1.1.x (вызовы сверены с заголовками C:\rrs-deps): готового
// узла частиц в VSG нет, поэтому система собрана из базовых примитивов
// по образцу graphics::PostProcessChain:
//
//   StateGroup
//   |- BindGraphicsPipeline (собственный GraphicsPipeline: GLSL-шейдеры
//   |  из строк, blending alpha, depth-write off)
//   |- ParticleMatrices (push-константы projection/modelView из
//   |  vsg::Camera, читаются В МОМЕНТ записи кадра)
//   |- vsg::VertexDraw (vertexCount = max_particles * 6, привязки —
//      динамические массивы vec4)
//
// Каждая частица — квад-биллборд: ориентация на камеру выполняется
// ВЕРШИННЫМ шейдером (смещение в пространстве вида по осям right/up
// из modelView), поэтому CPU каждый кадр заполняет только два
// массива vec4 (позиция+размер, цвет+альфа):
//
//   - vsg::vec4Array с properties.dataVariance = vsg::DYNAMIC_DATA
//     и вызовом dirty() после обновления: vsg::TransferTask вьювера
//     (назначается viewer->compile()) переносит их в GPU перед обходом
//     записи (механизм описан в комментарии vsg/app/TransferTask.h);
//   - мёртвые частицы — вырожденные квады (размер 0), отдельный
//     буфер/индексный список не нужен.
//
// Обновление CPU-стороны (step) дёшево: пул 64 частицы перебирается
// линейно, GPU передаются только живые значения.
//------------------------------------------------------------------------------
namespace graphics
{

/// CPU-пул спрайтов-биллбордов
class ParticleSystem final
{
public:
    /// Параметры одной частицы при рождении (мировые координаты)
    struct Spawn
    {
        vsg::vec3 position = vsg::vec3(0.0f, 0.0f, 0.0f);
        vsg::vec3 velocity = vsg::vec3(0.0f, 0.0f, 0.0f);
        vsg::vec3 color = vsg::vec3(1.0f, 1.0f, 1.0f);
        float alpha = 0.5f;         ///< Пиковая альфа (после fade in/out)
        float size_begin = 0.5f;    ///< Размер при рождении, м
        float size_end = 3.0f;      ///< Размер к концу жизни, м
        float lifetime = 4.0f;      ///< Время жизни, с
    };

    /// max_particles — размер пула (64 для дыма/брызг достаточно);
    /// camera — источник матриц для GPU-биллборда (без неё система
    /// не создаётся)
    ParticleSystem(std::size_t max_particles, vsg::ref_ptr<vsg::Camera> camera);
    ~ParticleSystem();

    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    /// Узел сцены (StateGroup с пайплайном + VertexDraw). Добавить
    /// в граф ДО viewer->compile(), чтобы буферы попали в TransferTask
    vsg::ref_ptr<vsg::Node> getNode() const;

    /// Заспавнить частицу; false — пул заполнен (свободных слотов нет)
    bool emitParticles(const Spawn& spawn);

    /// Шаг симуляции: движение, время жизни, fade in/out, рост размера,
    /// запись живых частиц в динамические массивы + dirty()
    void step(double dt);

    /// Число живых частиц (диагностика)
    std::size_t aliveCount() const { return alive_count_; }

private:
    struct Particle
    {
        vsg::vec3 position = vsg::vec3(0.0f, 0.0f, 0.0f);
        vsg::vec3 velocity = vsg::vec3(0.0f, 0.0f, 0.0f);
        vsg::vec3 color = vsg::vec3(1.0f, 1.0f, 1.0f);
        float alpha = 0.0f;
        float size_begin = 0.0f;
        float size_end = 0.0f;
        float lifetime = 0.0f;
        float age = 0.0f;           ///< >= lifetime — частица мертва
    };

    std::size_t max_particles_ = 0;
    std::size_t alive_count_ = 0;
    std::size_t emit_cursor_ = 0;               ///< Кольцевой курсор поиска свободного слота
    std::vector<Particle> particles_;

    vsg::ref_ptr<vsg::Node> node_;
    vsg::ref_ptr<vsg::vec4Array> positions_;    ///< xyz — позиция, w — размер
    vsg::ref_ptr<vsg::vec4Array> colors_;       ///< rgb — цвет, a — альфа
};

} // namespace graphics

#endif // GRAPHICS_PARTICLES_H
