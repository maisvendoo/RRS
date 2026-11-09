#include "export-pipeline.h"

#include <QDate>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QTextStream>

//------------------------------------------------------------------------------
//
//  Сборка пакета ПС: конфигурация XML, копия модели, инструкция README.
//  Механизма загрузки data-only ПС в движке нет (нужен C++-модуль,
//  как у addons/vl60/vl60k), поэтому пакет дополняется инструкцией
//  по подключению — это честно отражено в README (ТЗ п.28-29).
//
//------------------------------------------------------------------------------
namespace
{

/// Расширения ресурсов, сопровождающих glTF-модель
bool isResourceSuffix(const QString& suffix)
{
    static const QStringList suffixes =
    {
        QStringLiteral("bin"),
        QStringLiteral("png"),
        QStringLiteral("jpg"),
        QStringLiteral("jpeg"),
        QStringLiteral("bmp"),
        QStringLiteral("tga"),
        QStringLiteral("webp"),
        QStringLiteral("ktx2"),
        QStringLiteral("dds")
    };

    return suffixes.contains(suffix.toLower());
}

/// Скопировать файл с перезаписью
bool copyFileOverwrite(const QString& source, const QString& target,
                       QString* error)
{
    if (QFile::exists(target) && !QFile::remove(target))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Не удалось перезаписать файл: %1")
                         .arg(target);
        }

        return false;
    }

    if (!QFile::copy(source, target))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Не удалось скопировать: %1 -> %2")
                         .arg(source, target);
        }

        return false;
    }

    return true;
}

/// Собрать пути звуковых файлов из секций <Sound> конфигурации:
/// атрибут File каждой секции + вариации Variants (через ';')
QStringList soundFilesFromConfig(const QString& config_xml)
{
    QStringList files;

    QDomDocument config_doc;

    if (!config_doc.setContent(config_xml.toUtf8()))
    {
        return files;
    }

    const QDomElement root = config_doc.documentElement();

    if (root.isNull())
    {
        return files;
    }

    for (QDomNode node = root.firstChild(); !node.isNull();
         node = node.nextSibling())
    {
        if (!node.isElement() ||
            node.toElement().tagName() != QStringLiteral("Sound"))
        {
            continue;
        }

        const QDomElement element = node.toElement();

        const QString file = element.attribute(
                    QStringLiteral("File")).trimmed();

        if (!file.isEmpty())
        {
            files << file;
        }

        const QString variants = element.attribute(
                    QStringLiteral("Variants"));

        for (const QString& variant :
             variants.split(';', Qt::SkipEmptyParts))
        {
            const QString trimmed = variant.trimmed();

            if (!trimmed.isEmpty())
            {
                files << trimmed;
            }
        }
    }

    files.removeDuplicates();

    return files;
}

/// Текст инструкции по подключению пакета
QString readmeText(const QString& vehicle_name,
                   const SceneModel::CollisionParams& collision,
                   const std::vector<PhysPoint>& points)
{
    QString text;
    QTextStream stream(&text);

    stream << "ПАКЕТ ПОДВИЖНОГО СОСТАВА RUSSIAN RAILWAY SIMULATOR\n";
    stream << "Имя: " << vehicle_name << "\n";
    stream << "Создан: " << QDate::currentDate().toString(Qt::ISODate);
    stream << " программой RRS Rolling Stock Creator\n\n";

    stream << "СОСТАВ ПАКЕТА\n";
    stream << "  " << vehicle_name << ".xml — конфигурация ПС ";
    stream << "(секции [Vehicle], [Cabine], [Collision] и др.)\n";
    stream << "  model/ — glTF-модель и её ресурсы\n";
    stream << "  sounds/ — файлы звуков из секций <Sound> конфигурации\n\n";

    stream << "ПОДКЛЮЧЕНИЕ К ИГРЕ\n";
    stream << "1. Конфигурацию скопируйте в <корень игры>/cfg/vehicles/";
    stream << vehicle_name << "/" << vehicle_name << ".xml\n";
    stream << "2. Содержимое model/ скопируйте в <корень игры>/data/models/";
    stream << vehicle_name << "/\n";
    stream << "   Путь к модели в секции [Model] конфигурации указывается ";
    stream << "относительно data/models\n";
    stream << "   (например: " << vehicle_name << "/model.gltf).\n";
    stream << "3. ВАЖНО: физика и тормозное оборудование ПЕ в RRS ";
    stream << "загружаются C++-модулем\n";
    stream << "   подвижной единицы (образец: addons/vl60/vl60k — ";
    stream << "посмотрите структуру модуля).\n";
    stream << "   Только XML + модель движок напрямую НЕ загружает, ";
    stream << "поэтому кнопка\n";
    stream << "   «Запустить в игре» в редакторе не предусмотрена.\n\n";

    stream << "КОЛЛИЗИИ\n";
    if (collision.valid)
    {
        stream << "  Секция [Collision] конфигурации сгенерирована по AABB ";
        stream << "мешей с ролями\n";
        stream << "  Body/Bogie/Wheel: кузов и тележки — box, колёсные пары — ";
        stream << "cylinder.\n";
        stream << "  Формат читает simulator/vehicle/src/vehicle-collision.cpp";
        stream << " (VehicleCollision::loadConfig).\n";
        stream << "  Для генерации collision-меша используйте инструмент ";
        stream << "tools/collider-gen.\n";
    }
    else
    {
        stream << "  Роли Body/Bogie/Wheel не назначены — секция [Collision] ";
        stream << "не сгенерирована.\n";
        stream << "  Назначьте роли во вкладке «Модель» и повторите экспорт";
        stream << " (ТЗ п.19).\n";
    }
    stream << "\n";

    stream << "ФИЗИЧЕСКИЕ ТОЧКИ\n";
    int camera_points = 0;

    for (const PhysPoint& point : points)
    {
        if (point.type == PhysPointType::Camera)
        {
            ++camera_points;
        }
    }

    stream << "  Точек камер (позиции машиниста) экспортировано в [Cabine]: ";
    stream << camera_points << "\n";
    stream << "  Остальные точки (" << (static_cast<int>(points.size()) - camera_points);
    stream << " шт.) хранятся в проекте .trainproject;\n";
    stream << "  движок читает из конфигурации ПС только [Cabine] ";
    stream << "(DriverPos/DriverDir — см.\n";
    stream << "  VehicleExterior::load_cabine_positions и [MassCenter]).\n\n";

    stream << "LOD\n";
    stream << "  Генерация LOD-мешей не выполняется (ТЗ п.20): ";
    stream << "используйте инструменты\n";
    stream << "  tools/collider-gen и tools/dmd2gltf из исходников RRS.\n";
    stream << "  Варианты lod1/lod2 рядом с моделью можно просмотреть ";
    stream << "переключателем LOD\n";
    stream << "  во вкладке «Модель» редактора.\n";

    return text;
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
ExportResult ExportPipeline::exportPackage(const QString& target_dir,
                                           const QString& vehicle_name,
                                           const QString& config_xml,
                                           const QString& model_path,
                                           const SceneModel::CollisionParams& collision,
                                           const std::vector<PhysPoint>& points,
                                           const QString& sounds_dir)
{
    ExportResult result;

    if (vehicle_name.isEmpty())
    {
        result.warnings << QStringLiteral("Не задано имя ПС");
        return result;
    }

    // Папка пакета: <target>/<имя>/
    const QString package_dir =
            QDir(target_dir).absoluteFilePath(vehicle_name);

    if (!QDir().mkpath(package_dir))
    {
        result.warnings << QStringLiteral("Не удалось создать папку: %1")
                               .arg(package_dir);
        return result;
    }

    result.packageDir = package_dir;

    // 1. Конфигурация XML
    const QString config_path =
            QDir(package_dir).absoluteFilePath(
                vehicle_name + QStringLiteral(".xml"));

    QFile config_file(config_path);

    if (!config_file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                          QIODevice::Text))
    {
        result.warnings << QStringLiteral("Не удалось записать: %1")
                               .arg(config_path);
        return result;
    }

    config_file.write(config_xml.toUtf8());
    config_file.close();
    result.messages << QStringLiteral("Записана конфигурация: %1")
                          .arg(config_path);

    // 2. Копия модели и её ресурсов
    if (!model_path.isEmpty())
    {
        if (!QFile::exists(model_path))
        {
            result.warnings << QStringLiteral("Файл модели не найден: %1")
                                   .arg(model_path);
        }
        else
        {
            const QString model_dir_path =
                    QDir(package_dir).absoluteFilePath(
                        QStringLiteral("model"));

            if (!QDir().mkpath(model_dir_path))
            {
                result.warnings << QStringLiteral("Не удалось создать папку: %1")
                                       .arg(model_dir_path);
                return result;
            }

            const QFileInfo model_info(model_path);
            const QDir source_dir = model_info.absoluteDir();

            // Сама модель
            QString copy_error;

            if (!copyFileOverwrite(
                        model_path,
                        QDir(model_dir_path).absoluteFilePath(
                            model_info.fileName()), &copy_error))
            {
                result.warnings << copy_error;
            }
            else
            {
                result.messages << QStringLiteral("Скопирована модель: %1")
                                      .arg(model_info.fileName());
            }

            // Ресурсы модели (.bin, текстуры) и LOD-варианты
            const QStringList entries =
                    source_dir.entryList(QDir::Files, QDir::Name);

            for (const QString& entry : entries)
            {
                const QFileInfo entry_info(entry);
                const QString suffix = entry_info.suffix().toLower();

                const bool is_model = (suffix == QLatin1String("gltf")) ||
                                      (suffix == QLatin1String("glb"));

                const bool lod_variant =
                        is_model && entry.contains(QLatin1String("lod"),
                                                   Qt::CaseInsensitive) &&
                        entry != model_info.fileName();

                const bool is_resource = isResourceSuffix(suffix);

                if (!is_resource && !lod_variant)
                {
                    continue;
                }

                // Файлы с именем модели (например, model.bin) копируем
                // как ресурсы, саму модель не дублируем
                if (entry == model_info.fileName())
                {
                    continue;
                }

                if (copyFileOverwrite(
                            source_dir.absoluteFilePath(entry),
                            QDir(model_dir_path).absoluteFilePath(entry),
                            &copy_error))
                {
                    result.messages << QStringLiteral("Скопирован файл: %1")
                                          .arg(entry);
                }
                else
                {
                    result.warnings << copy_error;
                }
            }
        }
    }
    else
    {
        result.warnings << QStringLiteral("Модель не задана — пакет "
                                          "содержит только конфигурацию");
    }

    // 2.5 Звуки из секций <Sound> (промт п.28): файлы копируются
    // в <пакет>/sounds/ с сохранением имён; относительные пути
    // ищутся в папке звуков (поле «Папка звуков» / папка конфига)
    {
        const QStringList sound_files = soundFilesFromConfig(config_xml);

        if (!sound_files.isEmpty())
        {
            const QString sounds_dir_path =
                    QDir(package_dir).absoluteFilePath(
                        QStringLiteral("sounds"));

            if (!QDir().mkpath(sounds_dir_path))
            {
                result.warnings << QStringLiteral("Не удалось создать папку: %1")
                                       .arg(sounds_dir_path);
            }
            else
            {
                QSet<QString> copied;

                for (const QString& sound_file : sound_files)
                {
                    QFileInfo source_info(sound_file);

                    if (!source_info.isAbsolute())
                    {
                        // Относительный путь — от папки звуков
                        source_info = QFileInfo(
                                    QDir(sounds_dir).absoluteFilePath(
                                        sound_file));
                    }

                    const QString target_name = source_info.fileName();

                    if (copied.contains(target_name))
                    {
                        continue;
                    }

                    if (!source_info.exists())
                    {
                        result.warnings << QStringLiteral(
                                            "Файл звука не найден: %1 "
                                            "(поиск в %2)")
                                            .arg(sound_file, sounds_dir);
                        continue;
                    }

                    QString copy_error;

                    if (copyFileOverwrite(
                                source_info.absoluteFilePath(),
                                QDir(sounds_dir_path).absoluteFilePath(
                                    target_name), &copy_error))
                    {
                        copied.insert(target_name);
                        result.messages << QStringLiteral(
                                            "Скопирован звук: %1")
                                            .arg(target_name);
                    }
                    else
                    {
                        result.warnings << copy_error;
                    }
                }
            }
        }
    }

    // 3. Инструкция по подключению
    const QString readme_path =
            QDir(package_dir).absoluteFilePath(QStringLiteral("README.txt"));

    QFile readme_file(readme_path);

    if (readme_file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                         QIODevice::Text))
    {
        readme_file.write(readmeText(vehicle_name, collision, points).toUtf8());
        readme_file.close();
        result.messages << QStringLiteral("Записана инструкция: %1")
                              .arg(readme_path);
    }
    else
    {
        result.warnings << QStringLiteral("Не удалось записать: %1")
                               .arg(readme_path);
    }

    result.success = true;

    return result;
}
