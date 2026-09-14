#include "editor/TrackProfile.h"

#include "editor/EditorContext.h"
#include "editor/RouteObject.h"

#include <Journal.h>

#include <CfgReader.h>

#include <QFile>
#include <QString>
#include <QStringList>
#include <QXmlStreamWriter>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <mutex>
#include <utility>
#include <vector>

/// Формат файла track-edit.conf:
///
/// <Config>
///     <Track name="ИмяТраектории">
///         <RailVariant>Р65</RailVariant>
///         <SleeperVariant>жб</SleeperVariant>
///         <BallastVariant>щебень</BallastVariant>
///         <HiddenMeshes>false false false</HiddenMeshes>
///         <MarkJoints>false</MarkJoints>
///         <BeginElevation>0.0</BeginElevation>
///         <EndElevation>0.0</EndElevation>
///         <GradePromille>0.0</GradePromille>
///         <GradeDecimal>0.0</GradeDecimal>
///         <SpeedLimit>80.0</SpeedLimit>
///         <VerticalCurve>auto</VerticalCurve>
///         <TransitionLength>0.0</TransitionLength>
///     </Track>
///     <Generated Kind="CatenaryPole" Traj="..." Coord="0.0" Side="right"/>
///     <Generated Kind="TrackMesh" Traj="..." From="0.0" To="100.0"/>
///     <Generated Kind="Trees" Traj="..." Side="right" PerKm="100.0"
///               Min="10.0" Max="30.0"/>
///     <Generated Kind="Water" X="0.0" Y="0.0" SizeX="200.0"
///               SizeY="200.0" Level="0.5"/>
///     <Generated Kind="Crossing" Traj="..." Coord="0.0"/>
///     <Generated Kind="Road" Traj="..." Coord="0.0" Len="50.0"/>
///     <Generated Kind="Embankment" Traj="..." From="0.0" To="100.0"
///               Hgt="2.0" Wid="4.6"/>
///     <Generated Kind="Cutting" Traj="..." From="0.0" To="100.0"
///               Hgt="2.0" Wid="8.0"/>
///     <Generated Kind="Ditch" Traj="..." From="0.0" To="100.0"
///               Side="right" Hgt="0.8" Wid="1.0"/>
///     <Layer Object="метка x y z" Name="слой"/>
///     <Prefab Name="префаб" Object="метка tx ty tz rx ry rz sx sy sz"/>
///     <ProposedTrack Name="путь" Points="x y z;x y z;..."/>
/// </Config>

double TrackProfile::auto_vertical_curve_radius() const
{
    // Таблица выбора радиуса вертикальной кривой по скорости
    if (speed_limit <= 60.0)
    {
        return 2000.0;
    }

    if (speed_limit <= 80.0)
    {
        return 6000.0;
    }

    if (speed_limit <= 120.0)
    {
        return 15000.0;
    }

    return 25000.0;
}

void TrackProfile::update_end_elevation(double length)
{
    end_elevation = begin_elevation + length * grade_decimal;
}

void TrackProfile::update_grade(double length)
{
    if (std::abs(length) < 1.0e-9)
    {
        return;
    }

    grade_decimal = (end_elevation - begin_elevation) / length;
    grade_promille = grade_decimal * 1000.0;
}

bool load_track_profiles(const std::string& path, TrackProfiles& profiles)
{
    // Совместимая обёртка: секции Generated/Layer/Prefab/ProposedTrack
    // игнорируются
    std::vector<GeneratedConfig> generated;
    std::vector<LayerConfig> layers;
    std::vector<PrefabConfig> prefabs;
    std::vector<ProposedTrackConfig> proposed_tracks;

    return load_track_edit_data(path, profiles, generated, layers, prefabs,
        proposed_tracks);
}

bool load_track_edit_data(const std::string& path, TrackProfiles& profiles,
    std::vector<GeneratedConfig>& generated,
    std::vector<LayerConfig>& layers,
    std::vector<PrefabConfig>& prefabs,
    std::vector<ProposedTrackConfig>& proposed_tracks)
{
    profiles.clear();
    generated.clear();
    layers.clear();
    prefabs.clear();
    proposed_tracks.clear();

    // Нет файла - нет профилей (новый маршрут), это не ошибка
    if (!std::filesystem::exists(path))
    {
        return true;
    }

    CfgReader cfg;

    if (!cfg.load(QString::fromStdString(path)))
    {
        Journal::instance()->error(QString("Failed to load %1")
            .arg(path.c_str()));

        return false;
    }

    QDomNode section = cfg.getFirstSection("Track");

    while (!section.isNull())
    {
        const QString name = section.toElement().attribute("name");

        if (!name.isEmpty())
        {
            TrackProfile profile;

            QString value;

            if (cfg.getString(section, "RailVariant", value))
            {
                profile.rail_variant = value.toStdString();
            }

            if (cfg.getString(section, "SleeperVariant", value))
            {
                profile.sleeper_variant = value.toStdString();
            }

            if (cfg.getString(section, "BallastVariant", value))
            {
                profile.ballast_variant = value.toStdString();
            }

            if (cfg.getString(section, "HiddenMeshes", value))
            {
                // Три флага через пробел: рельс, шпала, балласт
                const QStringList flags = value.simplified().split(' ');

                if (flags.size() >= 3)
                {
                    profile.rail_hidden = flags.at(0) == "true";
                    profile.sleeper_hidden = flags.at(1) == "true";
                    profile.ballast_hidden = flags.at(2) == "true";
                }
            }

            // Отметка стыков рельсов жёлтыми поперечинами (каждые 25 м)
            if (cfg.getString(section, "MarkJoints", value))
            {
                profile.mark_joints = value.simplified() == "true";
            }

            cfg.getDouble(section, "BeginElevation", profile.begin_elevation);
            cfg.getDouble(section, "EndElevation", profile.end_elevation);

            // Тысячные - основное представление, доли синхронизируются
            const bool has_promille = cfg.getDouble(section, "GradePromille",
                profile.grade_promille);

            const bool has_decimal = cfg.getDouble(section, "GradeDecimal",
                profile.grade_decimal);

            if (!has_promille && has_decimal)
            {
                profile.grade_promille = profile.grade_decimal * 1000.0;
            }

            if (has_promille)
            {
                profile.grade_decimal = profile.grade_promille / 1000.0;
            }

            cfg.getDouble(section, "SpeedLimit", profile.speed_limit);

            if (cfg.getString(section, "VerticalCurve", value))
            {
                const QString mode = value.simplified();

                if (mode.toLower() == "auto")
                {
                    profile.vertical_curve_auto = true;
                }
                else
                {
                    profile.vertical_curve_auto = false;
                    profile.vertical_curve_radius = mode.toDouble();
                }
            }

            cfg.getDouble(section, "TransitionLength",
                profile.transition_length);

            profiles.emplace(name.toStdString(), profile);
        }

        section = cfg.getNextSection();
    }

    // Секции <Generated Kind="..." Traj="..." Coord="..." Side="..."
    // Label="..." Len="..." Wid="..." Hgt="..." From="..." To="..."
    // PerKm="..." Min="..." Max="..." X="..." Y="..." SizeX="..."
    // SizeY="..." Level="..."/> - сгенерированный обвес пути
    // (окно «Путь» -> «Генерация»). Traj может быть пустой
    // (Kind == Water не привязан к траектории)
    section = cfg.getFirstSection("Generated");

    while (!section.isNull())
    {
        const QDomElement element = section.toElement();

        GeneratedConfig item;
        item.kind = element.attribute("Kind").toStdString();
        item.trajectory = element.attribute("Traj").toStdString();
        item.coord = element.attribute("Coord").toDouble();
        item.side = element.attribute("Side", "right").toStdString();
        item.label = element.attribute("Label").toStdString();
        item.length = element.attribute("Len").toDouble();
        item.width = element.attribute("Wid").toDouble();
        item.height = element.attribute("Hgt").toDouble();

        // Интервал вдоль траектории (Kind == TrackMesh)
        item.from = element.attribute("From").toDouble();
        item.to = element.attribute("To").toDouble();

        // Плотность и отступы деревьев (Kind == Trees)
        item.per_km = element.attribute("PerKm").toDouble();
        item.offset_min = element.attribute("Min").toDouble();
        item.offset_max = element.attribute("Max").toDouble();

        // Вода: центр и уровень зеркала (Kind == Water)
        item.pos_x = element.attribute("X").toDouble();
        item.pos_y = element.attribute("Y").toDouble();
        item.level = element.attribute("Level").toDouble();

        if (element.hasAttribute("SizeX"))
        {
            item.length = element.attribute("SizeX").toDouble();
        }

        if (element.hasAttribute("SizeY"))
        {
            item.width = element.attribute("SizeY").toDouble();
        }

        // Траектория обязательна для элементов вдоль пути; для Water
        // и прочих "точечных" элементов она может быть пустой
        if (!item.kind.empty())
        {
            generated.push_back(std::move(item));
        }

        section = cfg.getNextSection();
    }

    // Секции <Layer Object="метка x y z" Name="..."/> - слои объектов.
    // В атрибуте Object последние три токена - позиция объекта,
    // остальное - метка
    section = cfg.getFirstSection("Layer");

    while (!section.isNull())
    {
        const QString object = section.toElement().attribute("Object");
        const QString name = section.toElement().attribute("Name");

        if (!object.isEmpty() && !name.isEmpty())
        {
            const QStringList tokens = object.simplified().split(' ');

            if (tokens.size() >= 4)
            {
                LayerConfig layer_config;
                layer_config.name = name.toStdString();
                layer_config.object_label = tokens.mid(0,
                    tokens.size() - 3).join(' ').toStdString();
                layer_config.position = vsg::dvec3{
                    tokens.at(tokens.size() - 3).toDouble(),
                    tokens.at(tokens.size() - 2).toDouble(),
                    tokens.at(tokens.size() - 1).toDouble()};

                layers.push_back(std::move(layer_config));
            }
        }

        section = cfg.getNextSection();
    }

    // Секции <Prefab Name="..." Object="метка tx ty tz rx ry rz
    // sx sy sz"/> - префабы объектов (окно «Префабы»). В атрибуте
    // Object последние девять токенов - позиция, поворот и масштаб
    // объекта, остальное - метка (как у слоёв)
    section = cfg.getFirstSection("Prefab");

    while (!section.isNull())
    {
        const QString name = section.toElement().attribute("Name");
        const QString object = section.toElement().attribute("Object");

        if (!name.isEmpty() && !object.isEmpty())
        {
            const QStringList tokens = object.simplified().split(' ');

            if (tokens.size() >= 10)
            {
                PrefabConfig prefab_config;
                prefab_config.name = name.toStdString();
                prefab_config.object_label = tokens.mid(0,
                    tokens.size() - 9).join(' ').toStdString();
                prefab_config.position = vsg::dvec3{
                    tokens.at(tokens.size() - 9).toDouble(),
                    tokens.at(tokens.size() - 8).toDouble(),
                    tokens.at(tokens.size() - 7).toDouble()};
                prefab_config.rotation_deg = vsg::dvec3{
                    tokens.at(tokens.size() - 6).toDouble(),
                    tokens.at(tokens.size() - 5).toDouble(),
                    tokens.at(tokens.size() - 4).toDouble()};
                prefab_config.scale = vsg::dvec3{
                    tokens.at(tokens.size() - 3).toDouble(),
                    tokens.at(tokens.size() - 2).toDouble(),
                    tokens.at(tokens.size() - 1).toDouble()};

                prefabs.push_back(std::move(prefab_config));
            }
        }

        section = cfg.getNextSection();
    }

    // Секции <ProposedTrack Name="..." Points="x y z;x y z;..."/> -
    // новые пути инструментом «Новый путь» (клавиша N): опорные точки
    // сплайна Catmull-Rom в мировых координатах, по ним при
    // восстановлении строится геометрия пути
    section = cfg.getFirstSection("ProposedTrack");

    while (!section.isNull())
    {
        const QString name = section.toElement().attribute("Name");
        const QString points = section.toElement().attribute("Points");

        if (!name.isEmpty() && !points.isEmpty())
        {
            ProposedTrackConfig track_config;
            track_config.name = name.toStdString();

            // Точки через ';', внутри точки координаты через пробел
            const QStringList point_tokens = points.simplified().split(';');

            for (const QString& point_token : point_tokens)
            {
                const QStringList coords =
                    point_token.simplified().split(' ');

                if (coords.size() >= 3)
                {
                    track_config.points.push_back(vsg::dvec3{
                        coords.at(0).toDouble(),
                        coords.at(1).toDouble(),
                        coords.at(2).toDouble()});
                }
            }

            if (track_config.points.size() >= 2)
            {
                proposed_tracks.push_back(std::move(track_config));
            }
        }

        section = cfg.getNextSection();
    }

    return true;
}

bool save_track_profiles(const std::string& path, const TrackProfiles& profiles)
{
    // Совместимая обёртка: секции Generated/Layer/Prefab/ProposedTrack
    // не пишутся
    return save_track_edit_data(path, profiles,
        std::vector<GeneratedConfig>(), std::vector<LayerConfig>(),
        std::vector<PrefabConfig>(), std::vector<ProposedTrackConfig>());
}

bool save_track_edit_data(const std::string& path,
    const TrackProfiles& profiles,
    const std::vector<GeneratedConfig>& generated,
    const std::vector<LayerConfig>& layers,
    const std::vector<PrefabConfig>& prefabs,
    const std::vector<ProposedTrackConfig>& proposed_tracks)
{
    QFile file(QString::fromStdString(path));

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |
                   QIODevice::Text))
    {
        Journal::instance()->error(QString("Failed to open %1 for writing")
            .arg(path.c_str()));

        return false;
    }

    QXmlStreamWriter writer(&file);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(4);

    writer.writeStartDocument();

    writer.writeStartElement("Config");

    for (const auto& [name, profile] : profiles)
    {
        writer.writeStartElement("Track");
        writer.writeAttribute("name", QString::fromStdString(name));

        writer.writeTextElement("RailVariant",
            QString::fromStdString(profile.rail_variant));

        writer.writeTextElement("SleeperVariant",
            QString::fromStdString(profile.sleeper_variant));

        writer.writeTextElement("BallastVariant",
            QString::fromStdString(profile.ballast_variant));

        writer.writeTextElement("HiddenMeshes", QString("%1 %2 %3")
            .arg(profile.rail_hidden ? "true" : "false")
            .arg(profile.sleeper_hidden ? "true" : "false")
            .arg(profile.ballast_hidden ? "true" : "false"));

        // Отметка стыков рельсов (геометрия TrackMesh)
        writer.writeTextElement("MarkJoints",
            profile.mark_joints ? "true" : "false");

        writer.writeTextElement("BeginElevation",
            QString::number(profile.begin_elevation, 'f', 3));

        writer.writeTextElement("EndElevation",
            QString::number(profile.end_elevation, 'f', 3));

        writer.writeTextElement("GradePromille",
            QString::number(profile.grade_promille, 'f', 2));

        writer.writeTextElement("GradeDecimal",
            QString::number(profile.grade_decimal, 'f', 5));

        writer.writeTextElement("SpeedLimit",
            QString::number(profile.speed_limit, 'f', 1));

        QString vertical_curve = "auto";

        if (!profile.vertical_curve_auto)
        {
            vertical_curve = QString::number(profile.vertical_curve_radius,
                'f', 1);
        }

        writer.writeTextElement("VerticalCurve", vertical_curve);

        writer.writeTextElement("TransitionLength",
            QString::number(profile.transition_length, 'f', 1));

        writer.writeEndElement();
    }

    // Сгенерированный обвес пути (окно «Путь» -> «Генерация»)
    for (const GeneratedConfig& item : generated)
    {
        writer.writeStartElement("Generated");
        writer.writeAttribute("Kind", QString::fromStdString(item.kind));
        writer.writeAttribute("Traj",
            QString::fromStdString(item.trajectory));
        writer.writeAttribute("Coord",
            QString::number(item.coord, 'f', 2));
        writer.writeAttribute("Side", QString::fromStdString(item.side));

        if (!item.label.empty())
        {
            writer.writeAttribute("Label",
                QString::fromStdString(item.label));
        }

        if (item.kind == "Platform")
        {
            writer.writeAttribute("Len",
                QString::number(item.length, 'f', 1));
            writer.writeAttribute("Wid",
                QString::number(item.width, 'f', 1));
            writer.writeAttribute("Hgt",
                QString::number(item.height, 'f', 2));
        }

        // Габариты терраформинга: высота/глубина и ширина
        // (Kind == Embankment / Cutting / Ditch)
        if (item.kind == "Embankment" || item.kind == "Cutting" ||
            item.kind == "Ditch")
        {
            writer.writeAttribute("Wid",
                QString::number(item.width, 'f', 2));
            writer.writeAttribute("Hgt",
                QString::number(item.height, 'f', 2));
        }

        // Дорога у переезда: длина ленты (Kind == Road)
        if (item.kind == "Road")
        {
            writer.writeAttribute("Len",
                QString::number(item.length, 'f', 1));
        }

        // Интервал генерации пути и терраформинга
        // (Kind == TrackMesh / Embankment / Cutting / Ditch)
        if (item.kind == "TrackMesh" || item.kind == "Embankment" ||
            item.kind == "Cutting" || item.kind == "Ditch")
        {
            writer.writeAttribute("From",
                QString::number(item.from, 'f', 1));
            writer.writeAttribute("To",
                QString::number(item.to, 'f', 1));
        }

        // Плотность и отступы деревьев (Kind == Trees)
        if (item.kind == "Trees")
        {
            writer.writeAttribute("PerKm",
                QString::number(item.per_km, 'f', 1));
            writer.writeAttribute("Min",
                QString::number(item.offset_min, 'f', 1));
            writer.writeAttribute("Max",
                QString::number(item.offset_max, 'f', 1));
        }

        // Вода: центр зеркала, размеры и уровень (Kind == Water)
        if (item.kind == "Water")
        {
            writer.writeAttribute("X",
                QString::number(item.pos_x, 'f', 2));
            writer.writeAttribute("Y",
                QString::number(item.pos_y, 'f', 2));
            writer.writeAttribute("SizeX",
                QString::number(item.length, 'f', 1));
            writer.writeAttribute("SizeY",
                QString::number(item.width, 'f', 1));
            writer.writeAttribute("Level",
                QString::number(item.level, 'f', 2));
        }

        writer.writeEndElement();
    }

    // Слои объектов (окно «Слои»); объект опознаётся по метке и позиции
    for (const LayerConfig& layer_config : layers)
    {
        writer.writeStartElement("Layer");
        writer.writeAttribute("Object", QString("%1 %2 %3 %4")
            .arg(QString::fromStdString(layer_config.object_label))
            .arg(layer_config.position.x, 0, 'f', 3)
            .arg(layer_config.position.y, 0, 'f', 3)
            .arg(layer_config.position.z, 0, 'f', 3));
        writer.writeAttribute("Name",
            QString::fromStdString(layer_config.name));
        writer.writeEndElement();
    }

    // Префабы объектов (окно «Префабы»): каждая запись - один объект
    // префаба (метка + полная трансформация на момент сохранения)
    for (const PrefabConfig& prefab_config : prefabs)
    {
        writer.writeStartElement("Prefab");
        writer.writeAttribute("Name",
            QString::fromStdString(prefab_config.name));
        writer.writeAttribute("Object", QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
            .arg(QString::fromStdString(prefab_config.object_label))
            .arg(prefab_config.position.x, 0, 'f', 3)
            .arg(prefab_config.position.y, 0, 'f', 3)
            .arg(prefab_config.position.z, 0, 'f', 3)
            .arg(prefab_config.rotation_deg.x, 0, 'f', 3)
            .arg(prefab_config.rotation_deg.y, 0, 'f', 3)
            .arg(prefab_config.rotation_deg.z, 0, 'f', 3)
            .arg(prefab_config.scale.x, 0, 'f', 3)
            .arg(prefab_config.scale.y, 0, 'f', 3)
            .arg(prefab_config.scale.z, 0, 'f', 3));
        writer.writeEndElement();
    }

    // Новые пути инструментом «Новый путь» (клавиша N): опорные точки
    // сплайна, по которым при восстановлении строится геометрия пути
    for (const ProposedTrackConfig& track_config : proposed_tracks)
    {
        QString points;

        for (std::size_t i = 0; i < track_config.points.size(); ++i)
        {
            if (i > 0)
            {
                points += ';';
            }

            points += QString("%1 %2 %3")
                .arg(track_config.points[i].x, 0, 'f', 3)
                .arg(track_config.points[i].y, 0, 'f', 3)
                .arg(track_config.points[i].z, 0, 'f', 3);
        }

        writer.writeStartElement("ProposedTrack");
        writer.writeAttribute("Name",
            QString::fromStdString(track_config.name));
        writer.writeAttribute("Points", points);
        writer.writeEndElement();
    }

    writer.writeEndElement();

    writer.writeEndDocument();

    file.close();

    return true;
}

bool load_track_edit_config(const std::string& path, EditorContext& context)
{
    context.pending_generated.clear();
    context.layer_configs.clear();
    context.prefab_configs.clear();
    context.proposed_tracks.clear();

    return load_track_edit_data(path, context.track_profiles,
        context.pending_generated, context.layer_configs,
        context.prefab_configs, context.proposed_tracks);
}

bool save_track_edit_config(const std::string& path, EditorContext& context)
{
    // Обвес: метаданные элементов, живущих в сцене (generated_items)
    std::vector<GeneratedConfig> generated;
    generated.reserve(context.generated_items.size());

    for (const GeneratedItem& item : context.generated_items)
    {
        generated.push_back(item.meta);
    }

    // Слои: только объекты, снятые со слоя "default"
    std::vector<LayerConfig> layers;

    {
        std::lock_guard<std::mutex> lock_guard(context.static_objects_mutex);

        for (const auto& object : context.static_objects)
        {
            if (object->layer.empty() || object->layer == "default")
            {
                continue;
            }

            LayerConfig layer_config;
            layer_config.object_label = object->label;
            layer_config.position = object->get_translation();
            layer_config.name = object->layer;

            layers.push_back(std::move(layer_config));
        }
    }

    return save_track_edit_data(path, context.track_profiles, generated,
        layers, context.prefab_configs, context.proposed_tracks);
}
