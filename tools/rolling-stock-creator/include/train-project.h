#ifndef TRAIN_PROJECT_H
#define TRAIN_PROJECT_H

#include "phys-points.h"

#include <QList>
#include <QString>

#include <vector>

//------------------------------------------------------------------------------
//
//  Проект .trainproject (ТЗ п.30): JSON-файл со ссылками на модель и
//  конфигурацию, ролями узлов, физическими точками и настройками экспорта.
//
//------------------------------------------------------------------------------
struct ProjectNode
{
    QString path;            ///< Путь узла в графе модели ("0/2/1")
    QString roleKey;         ///< Ключ игровой роли (mesh-roles.h)
    QString displayName;     ///< Display-имя объекта
    bool visible = true;     ///< Видимость меша
};

struct ExportSettings
{
    QString targetDir;       ///< Папка назначения пакета
    QString vehicleName;     ///< Имя ПС (имя пакета, по умолчанию addons/<имя>)
    bool copyModel = true;   ///< Копировать glTF-модель в пакет
};

class TrainProject
{
public:
    /// Прочитать проект из JSON-файла
    bool load(const QString& file_path, QString* error = nullptr);

    /// Сохранить проект в JSON-файл
    bool save(const QString& file_path, QString* error = nullptr) const;

    /// Путь к файлу проекта (после load/save)
    const QString& filePath() const;

    // --- Данные проекта ---
    QString modelPath;             ///< glTF-модель ПС
    int lodIndex = 0;              ///< Выбранный вариант LOD (превью)
    QString configPath;            ///< XML-конфиг характеристик ПС
    QString templateKey;           ///< Ключ шаблона проекта
    QList<ProjectNode> nodes;      ///< Роли/display-имена/видимость узлов
    std::vector<PhysPoint> points; ///< Физические точки
    ExportSettings exportSettings;

private:
    QString file_path_;
};

//------------------------------------------------------------------------------
//
//  Шаблоны новых проектов (ТЗ п.31)
//
//------------------------------------------------------------------------------
struct TemplateSpec
{
    QString key;         ///< Латинский ключ
    QString title;       ///< Русское название
    QString description; ///< Краткое описание
};

/// Список шаблонов
const QList<TemplateSpec>& projectTemplates();

/// Начальный XML-конфиг ПС по шаблону (числа — из конфигов RRS:
/// cfg/vehicles/vl60k, cfg/vehicles/Fr_hopper_RZD, cfg/vehicles/IMR_pass_rzd)
QString templateConfigXml(const QString& template_key);

/// Базовые физические точки, создаваемые по шаблону (ТЗ п.31:
/// «автоматически создаёт базовые точки»)
std::vector<PhysPoint> templatePoints(const QString& template_key);

#endif // TRAIN_PROJECT_H
