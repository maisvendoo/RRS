#include "train-project.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

//------------------------------------------------------------------------------
//
//  Проект .trainproject: открытие/сохранение JSON, шаблоны новых проектов.
//
//------------------------------------------------------------------------------
bool TrainProject::load(const QString& file_path, QString* error)
{
    QFile file(file_path);

    if (!file.open(QIODevice::ReadOnly))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Не удалось открыть файл: %1").arg(file_path);
        }

        return false;
    }

    const QByteArray raw = file.readAll();
    file.close();

    QJsonParseError parse_error;
    const QJsonDocument document =
            QJsonDocument::fromJson(raw, &parse_error);

    if (parse_error.error != QJsonParseError::NoError)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Ошибка разбора JSON: %1")
                         .arg(parse_error.errorString());
        }

        return false;
    }

    const QJsonObject root = document.object();

    modelPath = root.value(QStringLiteral("modelPath")).toString();
    lodIndex = root.value(QStringLiteral("lodIndex")).toInt(0);
    configPath = root.value(QStringLiteral("configPath")).toString();
    templateKey = root.value(QStringLiteral("template")).toString();

    nodes.clear();
    const QJsonValue nodes_value = root.value(QStringLiteral("nodes"));

    if (nodes_value.isArray())
    {
        for (const QJsonValue& value : nodes_value.toArray())
        {
            const QJsonObject object = value.toObject();
            ProjectNode node;
            node.path = object.value(QStringLiteral("path")).toString();
            node.roleKey = object.value(QStringLiteral("role")).toString();
            node.displayName =
                    object.value(QStringLiteral("displayName")).toString();
            node.visible =
                    object.value(QStringLiteral("visible")).toBool(true);
            nodes.push_back(node);
        }
    }

    points.clear();
    const QJsonValue points_value = root.value(QStringLiteral("points"));

    if (points_value.isArray())
    {
        for (const QJsonValue& value : points_value.toArray())
        {
            const QJsonObject object = value.toObject();
            PhysPoint point;
            point.name = object.value(QStringLiteral("name")).toString();
            point.type = pointTypeFromKey(
                        object.value(QStringLiteral("type")).toString());
            point.x = object.value(QStringLiteral("x")).toDouble(0.0);
            point.y = object.value(QStringLiteral("y")).toDouble(0.0);
            point.z = object.value(QStringLiteral("z")).toDouble(0.0);
            point.heading =
                    object.value(QStringLiteral("heading")).toDouble(0.0);
            points.push_back(point);
        }
    }

    const QJsonObject export_object = root.value(QStringLiteral("export")).toObject();
    exportSettings.targetDir =
            export_object.value(QStringLiteral("targetDir")).toString();
    exportSettings.vehicleName =
            export_object.value(QStringLiteral("vehicleName")).toString();
    exportSettings.copyModel =
            export_object.value(QStringLiteral("copyModel")).toBool(true);

    file_path_ = file_path;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool TrainProject::save(const QString& file_path, QString* error) const
{
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("modelPath"), modelPath);
    root.insert(QStringLiteral("lodIndex"), lodIndex);
    root.insert(QStringLiteral("configPath"), configPath);
    root.insert(QStringLiteral("template"), templateKey);

    QJsonArray nodes_array;

    for (const ProjectNode& node : nodes)
    {
        QJsonObject object;
        object.insert(QStringLiteral("path"), node.path);
        object.insert(QStringLiteral("role"), node.roleKey);
        object.insert(QStringLiteral("displayName"), node.displayName);
        object.insert(QStringLiteral("visible"), node.visible);
        nodes_array.append(object);
    }

    root.insert(QStringLiteral("nodes"), nodes_array);

    QJsonArray points_array;

    for (const PhysPoint& point : points)
    {
        QJsonObject object;
        object.insert(QStringLiteral("name"), point.name);
        object.insert(QStringLiteral("type"), pointTypeKey(point.type));
        object.insert(QStringLiteral("x"), point.x);
        object.insert(QStringLiteral("y"), point.y);
        object.insert(QStringLiteral("z"), point.z);
        object.insert(QStringLiteral("heading"), point.heading);
        points_array.append(object);
    }

    root.insert(QStringLiteral("points"), points_array);

    QJsonObject export_object;
    export_object.insert(QStringLiteral("targetDir"), exportSettings.targetDir);
    export_object.insert(QStringLiteral("vehicleName"),
                         exportSettings.vehicleName);
    export_object.insert(QStringLiteral("copyModel"), exportSettings.copyModel);
    root.insert(QStringLiteral("export"), export_object);

    QFile file(file_path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("Не удалось открыть файл для записи: %1")
                         .arg(file_path);
        }

        return false;
    }

    const QJsonDocument document(root);
    file.write(document.toJson(QJsonDocument::Indented));
    file.close();

    file_path_ = file_path;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QString& TrainProject::filePath() const
{
    return file_path_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const QList<TemplateSpec>& projectTemplates()
{
    static const QList<TemplateSpec> templates =
    {
        {
            QStringLiteral("passenger_car"),
            QStringLiteral("Пассажирский вагон"),
            QStringLiteral("Цельнометалкий пассажирский вагон "
                           "(значения — cfg/vehicles/IMR_pass_rzd)")
        },
        {
            QStringLiteral("freight_car"),
            QStringLiteral("Грузовая платформа"),
            QStringLiteral("Грузовой вагон (значения — "
                           "cfg/vehicles/Fr_hopper_RZD)")
        },
        {
            QStringLiteral("diesel_loco"),
            QStringLiteral("Тепловоз"),
            QStringLiteral("Тепловоз с электрической передачей")
        },
        {
            QStringLiteral("electric_loco"),
            QStringLiteral("Электровоз"),
            QStringLiteral("Электровоз (значения — cfg/vehicles/vl60k)")
        }
    };

    return templates;
}

namespace
{

/// Строка секции XML с одним ключом
QString tag(const QString& key, const QString& value)
{
    return QStringLiteral("\t\t<%1>%2</%1>\n").arg(key, value);
}

/// Секция кабины (позиция машиниста + направление взгляда)
QString cabineSection(const QString& pos, const QString& dir)
{
    return QStringLiteral("\t<Cabine>\n") +
           tag(QStringLiteral("DriverPos"), pos) +
           tag(QStringLiteral("DriverDir"), dir) +
           QStringLiteral("\t</Cabine>\n");
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString templateConfigXml(const QString& template_key)
{
    QString body;

    if (template_key == QLatin1String("passenger_car"))
    {
        // Числа из cfg/vehicles/IMR_pass_rzd/IMR_pass_rzd-11100.xml
        body += QStringLiteral("\t<Vehicle>\n");
        body += tag(QStringLiteral("EmptyMass"), QStringLiteral("56500"));
        body += tag(QStringLiteral("PayloadMass"), QStringLiteral("4000"));
        body += tag(QStringLiteral("Length"), QStringLiteral("25.0"));
        body += tag(QStringLiteral("WheelDiameter"), QStringLiteral("0.95"));
        body += tag(QStringLiteral("MainResist"), QStringLiteral("passcar"));
        body += tag(QStringLiteral("NumAxis"), QStringLiteral("4"));
        body += tag(QStringLiteral("WheelInertia"), QStringLiteral("100.0"));
        body += tag(QStringLiteral("SoundDir"), QStringLiteral("pass"));
        body += tag(QStringLiteral("CouplingModule"), QStringLiteral("sa3"));
        body += tag(QStringLiteral("CouplingConfig"), QStringLiteral("sa3"));
        body += tag(QStringLiteral("AirDistModule"), QStringLiteral("vr242"));
        body += tag(QStringLiteral("AirDistConfig"), QStringLiteral("vr242"));
        body += tag(QStringLiteral("ElectroAirDistModule"), QStringLiteral("evr305"));
        body += tag(QStringLiteral("ElectroAirDistConfig"), QStringLiteral("evr305"));
        body += tag(QStringLiteral("SupplyReservoirVolume"), QStringLiteral("0.078"));
        body += tag(QStringLiteral("BrakepipeAnglecockConfig"),
                    QStringLiteral("pneumo-anglecock-BP"));
        body += tag(QStringLiteral("BrakepipeHoseModule"), QStringLiteral("hose369a"));
        body += tag(QStringLiteral("BrakepipeHoseConfig"),
                    QStringLiteral("pneumo-hose-BP369a-passcar"));
        body += tag(QStringLiteral("BrakeMechConfig"),
                    QStringLiteral("carbrakes-mech-composite"));
        body += tag(QStringLiteral("GenReductorCoeff"), QStringLiteral("2.96"));
        body += QStringLiteral("\t</Vehicle>\n\n");

        body += cabineSection(QStringLiteral("2.0 12.0 2.2"),
                              QStringLiteral("30.0"));
        body += cabineSection(QStringLiteral("-2.0 -12.0 2.2"),
                              QStringLiteral("-150.0"));
    }
    else if (template_key == QLatin1String("freight_car"))
    {
        // Числа из cfg/vehicles/Fr_hopper_RZD/Fr_hopper_RZD-2851.xml
        body += QStringLiteral("\t<Vehicle>\n");
        body += tag(QStringLiteral("EmptyMass"), QStringLiteral("21300"));
        body += tag(QStringLiteral("PayloadMass"), QStringLiteral("70000"));
        body += tag(QStringLiteral("Length"), QStringLiteral("14.5"));
        body += tag(QStringLiteral("WheelDiameter"), QStringLiteral("0.95"));
        body += tag(QStringLiteral("MainResist"), QStringLiteral("default"));
        body += tag(QStringLiteral("NumAxis"), QStringLiteral("4"));
        body += tag(QStringLiteral("WheelInertia"), QStringLiteral("100.0"));
        body += tag(QStringLiteral("SoundDir"), QStringLiteral("freight"));
        body += tag(QStringLiteral("BrakepipeLeak"), QStringLiteral("3e-6"));
        body += tag(QStringLiteral("CouplingModule"), QStringLiteral("sa3"));
        body += tag(QStringLiteral("CouplingConfig"), QStringLiteral("sa3"));
        body += tag(QStringLiteral("AirDistModule"), QStringLiteral("vr483"));
        body += tag(QStringLiteral("AirDistConfig"), QStringLiteral("vr483"));
        body += tag(QStringLiteral("SupplyReservoirVolume"), QStringLiteral("0.078"));
        body += tag(QStringLiteral("SupplyReservoirLeak"), QStringLiteral("1e-6"));
        body += tag(QStringLiteral("BrakepipeAnglecockConfig"),
                    QStringLiteral("pneumo-anglecock-BP"));
        body += tag(QStringLiteral("BrakepipeHoseConfig"),
                    QStringLiteral("pneumo-hose-BP"));
        body += tag(QStringLiteral("BrakeAutomodeModule"), QStringLiteral("ar265"));
        body += tag(QStringLiteral("BrakeAutomodeConfig"), QStringLiteral("ar265"));
        body += tag(QStringLiteral("BrakeMechConfig"),
                    QStringLiteral("carbrakes-mech-composite"));
        body += QStringLiteral("\t</Vehicle>\n\n");

        body += cabineSection(QStringLiteral("2.0 5.4 2.2"),
                              QStringLiteral("30.0"));
        body += cabineSection(QStringLiteral("-2.0 -5.4 2.2"),
                              QStringLiteral("-150.0"));
    }
    else if (template_key == QLatin1String("electric_loco"))
    {
        // Числа из cfg/vehicles/vl60k/vl60k-1737.xml
        body += QStringLiteral("\t<Vehicle>\n");
        body += tag(QStringLiteral("EmptyMass"), QStringLiteral("136000"));
        body += tag(QStringLiteral("PayloadMass"), QStringLiteral("2000"));
        body += tag(QStringLiteral("Length"), QStringLiteral("21.0"));
        body += tag(QStringLiteral("WheelDiameter"), QStringLiteral("1.236"));
        body += tag(QStringLiteral("MainResist"), QStringLiteral("loco-resist"));
        body += tag(QStringLiteral("WheelRailFriction"), QStringLiteral("loco-AC"));
        body += tag(QStringLiteral("NumAxis"), QStringLiteral("6"));
        body += tag(QStringLiteral("WheelInertia"), QStringLiteral("100.0"));
        body += tag(QStringLiteral("SoundDir"), QStringLiteral("vl60"));
        body += tag(QStringLiteral("ReductorCoeff"), QStringLiteral("3.83"));
        body += tag(QStringLiteral("CouplingModule"), QStringLiteral("sa3"));
        body += tag(QStringLiteral("CouplingConfig"), QStringLiteral("sa3"));
        body += tag(QStringLiteral("BrakeCraneModule"), QStringLiteral("krm395"));
        body += tag(QStringLiteral("BrakeCraneConfig"), QStringLiteral("krm395"));
        body += tag(QStringLiteral("LocoCraneModule"), QStringLiteral("kvt254"));
        body += tag(QStringLiteral("LocoCraneConfig"), QStringLiteral("kvt254"));
        body += tag(QStringLiteral("AirDistModule"), QStringLiteral("vr483"));
        body += tag(QStringLiteral("AirDistConfig"), QStringLiteral("vr483"));
        body += QStringLiteral("\t</Vehicle>\n\n");

        body += cabineSection(QStringLiteral("0.8 8.4 3.2"),
                              QStringLiteral("0.0"));
        body += cabineSection(QStringLiteral("-0.8 -8.4 3.2"),
                              QStringLiteral("180.0"));
    }
    else if (template_key == QLatin1String("diesel_loco"))
    {
        // Тепловоз: тормозное оборудование как у ВЛ60к (krm395/kvt254/vr483),
        // масса/длина — типовой шестиосный тепловоз
        body += QStringLiteral("\t<Vehicle>\n");
        body += tag(QStringLiteral("EmptyMass"), QStringLiteral("126000"));
        body += tag(QStringLiteral("PayloadMass"), QStringLiteral("2000"));
        body += tag(QStringLiteral("Length"), QStringLiteral("17.4"));
        body += tag(QStringLiteral("WheelDiameter"), QStringLiteral("1.05"));
        body += tag(QStringLiteral("MainResist"), QStringLiteral("loco-resist"));
        body += tag(QStringLiteral("WheelRailFriction"), QStringLiteral("loco-AC"));
        body += tag(QStringLiteral("NumAxis"), QStringLiteral("6"));
        body += tag(QStringLiteral("WheelInertia"), QStringLiteral("100.0"));
        body += tag(QStringLiteral("SoundDir"), QStringLiteral("freight"));
        body += tag(QStringLiteral("ReductorCoeff"), QStringLiteral("4.41"));
        body += tag(QStringLiteral("CouplingModule"), QStringLiteral("sa3"));
        body += tag(QStringLiteral("CouplingConfig"), QStringLiteral("sa3"));
        body += tag(QStringLiteral("BrakeCraneModule"), QStringLiteral("krm395"));
        body += tag(QStringLiteral("BrakeCraneConfig"), QStringLiteral("krm395"));
        body += tag(QStringLiteral("LocoCraneModule"), QStringLiteral("kvt254"));
        body += tag(QStringLiteral("LocoCraneConfig"), QStringLiteral("kvt254"));
        body += tag(QStringLiteral("AirDistModule"), QStringLiteral("vr483"));
        body += tag(QStringLiteral("AirDistConfig"), QStringLiteral("vr483"));
        body += QStringLiteral("\t</Vehicle>\n\n");

        body += cabineSection(QStringLiteral("0.8 6.6 3.1"),
                              QStringLiteral("0.0"));
        body += cabineSection(QStringLiteral("-0.8 -6.6 3.1"),
                              QStringLiteral("180.0"));
    }
    else
    {
        // Пустой конфиг с обязательными ключами [Vehicle]
        body += QStringLiteral("\t<Vehicle>\n");
        body += tag(QStringLiteral("EmptyMass"), QStringLiteral("40000"));
        body += tag(QStringLiteral("PayloadMass"), QStringLiteral("60000"));
        body += tag(QStringLiteral("Length"), QStringLiteral("15.0"));
        body += tag(QStringLiteral("NumAxis"), QStringLiteral("4"));
        body += tag(QStringLiteral("WheelDiameter"), QStringLiteral("0.95"));
        body += QStringLiteral("\t</Vehicle>\n\n");
    }

    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                          "<Config>\n") + body + QStringLiteral("</Config>\n");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::vector<PhysPoint> templatePoints(const QString& template_key)
{
    std::vector<PhysPoint> points;

    auto add_point = [&points](const QString& name, PhysPointType type,
                               double x, double y, double z)
    {
        PhysPoint point;
        point.name = name;
        point.type = type;
        point.x = x;
        point.y = y;
        point.z = z;
        points.push_back(point);
    };

    if (template_key == QLatin1String("passenger_car"))
    {
        // Характерные точки пассажирского вагона длиной 25 м
        add_point(QStringLiteral("CouplerFwd"), PhysPointType::Coupler,
                  0.0, 12.5, 1.06);
        add_point(QStringLiteral("CouplerBwd"), PhysPointType::Coupler,
                  0.0, -12.5, 1.06);
        add_point(QStringLiteral("BrakeHoseFwd"), PhysPointType::BrakeHose,
                  0.0, 12.3, 1.25);
        add_point(QStringLiteral("BrakeHoseBwd"), PhysPointType::BrakeHose,
                  0.0, -12.3, 1.25);
        add_point(QStringLiteral("EndValveFwd"), PhysPointType::EndValve,
                  0.0, 12.4, 1.35);
        add_point(QStringLiteral("EndValveBwd"), PhysPointType::EndValve,
                  0.0, -12.4, 1.35);
        add_point(QStringLiteral("DoorFwdRight"), PhysPointType::PassengerDoor,
                  1.4, 8.0, 0.5);
        add_point(QStringLiteral("DoorBwdRight"), PhysPointType::PassengerDoor,
                  1.4, -8.0, 0.5);
    }
    else if (template_key == QLatin1String("freight_car"))
    {
        // Характерные точки грузового вагона длиной 14.5 м
        add_point(QStringLiteral("CouplerFwd"), PhysPointType::Coupler,
                  0.0, 7.25, 1.06);
        add_point(QStringLiteral("CouplerBwd"), PhysPointType::Coupler,
                  0.0, -7.25, 1.06);
        add_point(QStringLiteral("BrakeHoseFwd"), PhysPointType::BrakeHose,
                  0.0, 7.05, 1.25);
        add_point(QStringLiteral("BrakeHoseBwd"), PhysPointType::BrakeHose,
                  0.0, -7.05, 1.25);
        add_point(QStringLiteral("EndValveFwd"), PhysPointType::EndValve,
                  0.0, 7.15, 1.35);
        add_point(QStringLiteral("EndValveBwd"), PhysPointType::EndValve,
                  0.0, -7.15, 1.35);
    }
    else
    {
        // Локомотивы (длина ~17-21 м): сцепки, рукава, токоприёмник, камеры
        const double half_length =
                (template_key == QLatin1String("electric_loco")) ? 10.5 : 8.7;

        add_point(QStringLiteral("CouplerFwd"), PhysPointType::Coupler,
                  0.0, half_length, 1.06);
        add_point(QStringLiteral("CouplerBwd"), PhysPointType::Coupler,
                  0.0, -half_length, 1.06);
        add_point(QStringLiteral("BrakeHoseFwd"), PhysPointType::BrakeHose,
                  0.0, half_length - 0.2, 1.25);
        add_point(QStringLiteral("BrakeHoseBwd"), PhysPointType::BrakeHose,
                  0.0, -(half_length - 0.2), 1.25);
        add_point(QStringLiteral("CameraFwd"), PhysPointType::Camera,
                  0.8, half_length - 2.0, 3.1);
        add_point(QStringLiteral("CameraBwd"), PhysPointType::Camera,
                  -0.8, -(half_length - 2.0), 3.1);

        if (template_key == QLatin1String("electric_loco"))
        {
            add_point(QStringLiteral("PantographFwd"), PhysPointType::Pantograph,
                      0.0, 4.0, 4.6);
            add_point(QStringLiteral("PantographBwd"), PhysPointType::Pantograph,
                      0.0, -4.0, 4.6);
        }
        else
        {
            add_point(QStringLiteral("FuelFillLeft"), PhysPointType::Fuel,
                      -1.3, 3.0, 1.0);
            add_point(QStringLiteral("FuelFillRight"), PhysPointType::Fuel,
                      1.3, 3.0, 1.0);
        }
    }

    return points;
}
