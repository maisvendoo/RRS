#ifndef EXPORT_PIPELINE_H
#define EXPORT_PIPELINE_H

#include "phys-points.h"
#include "scene-model.h"

#include <QString>
#include <QStringList>

#include <vector>

//------------------------------------------------------------------------------
//
//  Пайплайн экспорта пакета ПС (ТЗ п.28-29): сборка пакета из модели,
//  конфигурации XML и инструкции по подключению.
//
//------------------------------------------------------------------------------
struct ExportResult
{
    bool success = false;      ///< Пакет собран
    QString packageDir;        ///< Папка созданного пакета
    QStringList messages;      ///< Что было сделано
    QStringList warnings;      ///< Предупреждения
};

class ExportPipeline
{
public:
    /// Собрать пакет ПС в папку target_dir/vehicle_name
    /// \param config_xml   готовый текст XML-конфига ПС
    /// \param model_path   путь к glTF-модели (пустой — модель не копируется)
    /// \param collision    параметры секции [Collision] (для отчёта)
    /// \param points       физические точки (для отчёта)
    static ExportResult exportPackage(const QString& target_dir,
                                      const QString& vehicle_name,
                                      const QString& config_xml,
                                      const QString& model_path,
                                      const SceneModel::CollisionParams& collision,
                                      const std::vector<PhysPoint>& points);
};

#endif // EXPORT_PIPELINE_H
