#include "math-funcs.h"
#include    <app.h>

#include    <QDir>
#include    <QDirIterator>
#include    <QStringList>
#include    <QFileInfo>

#include    <iostream>
#include    <fstream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Application::Application(int argc, char *argv[])
    : QCoreApplication(argc, argv)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Application::~Application()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int Application::exec()
{
    QString errorMessage = "";

    switch (parseCommandLine(errorMessage))
    {

    case CommandLineOK:
    {
        generate_topology(routeDir);
        return 0;
    }

    case CommandLineHelp:

        parser.showHelp();

    case CommandLineVersion:

        return 0;

    case CommandLineError:

        fputs(qPrintable(errorMessage), stderr);
        fputs("\n", stderr);
        return -1;
    }

    return 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Application::CommandLineResult Application::parseCommandLine(QString &errorMessage)
{
    QCommandLineOption optHelp = parser.addHelpOption();
    QCommandLineOption optVersion = parser.addVersionOption();

    QCommandLineOption optRouteDir(QStringList() << "r" << "route",
                                   QCoreApplication::translate("main", "Path to route's directory"),
                                   QCoreApplication::translate("main", "route directory path"));

    QCommandLineOption optFile(QStringList() << "f" << "file",
                               QCoreApplication::translate("main", "Output filename, default: trajectory"),
                               QCoreApplication::translate("main", "filename"),
                               QString("trajectory"));

    QCommandLineOption optRouteTrkNum(QStringList() << "t" << "trk",
                                      QCoreApplication::translate("main", "Select data: route1.trk or route2.trk, default: 1 (route1.trk)"),
                                      QCoreApplication::translate("main", "1 or 2"),
                                      QString("1"));

    QCommandLineOption optTrack(QStringList() << "i" << "id",
                                     QCoreApplication::translate("main", "Number of track"),
                                     QCoreApplication::translate("main", "track"));

    QCommandLineOption optBiasBegin(QStringList() << "d" << "distance",
                               QCoreApplication::translate("main", "Offset distance: >0 to right, <0 to left, default: 0.0"),
                               QCoreApplication::translate("main", "distance begin"),
                               QString("0.0"));

    QCommandLineOption optBiasEnd(QStringList() << "e" << "end",
                                  QCoreApplication::translate("main", "Offset distance: >0 to right, <0 to left, default: 7.5"),
                                  QCoreApplication::translate("main", "distance end"),
                                  QString("7.5"));

    parser.addOption(optRouteDir);
    parser.addOption(optFile);
    parser.addOption(optRouteTrkNum);
    parser.addOption(optTrack);
    parser.addOption(optBiasBegin);
    parser.addOption(optBiasEnd);

    if (!parser.parse(this->arguments()))
    {
        errorMessage = parser.errorText();
        return CommandLineError;
    }

    if (parser.isSet(optHelp))
        return CommandLineHelp;

    if (parser.isSet(optVersion))
        return CommandLineVersion;

    if (parser.isSet(optRouteDir))
    {
        routeDir = parser.value(optRouteDir);
        std::cout << "route: " << routeDir.toStdString() << std::endl;
    }

    if (parser.isSet(optFile))
    {
        filename = parser.value(optFile);
        std::cout << "file: " << filename.toStdString() << FILE_EXTENTION << std::endl;
    }

    if (parser.isSet(optRouteTrkNum))
    {
        QString tmp = parser.value(optRouteTrkNum);
        if (tmp == "1")
            trkfile = "route1.trk";
        if (tmp == "2")
            trkfile = "route2.trk";
        if (tmp == "-1")
            trkfile = "route2.trk";
        std::cout << "data: " << trkfile.toStdString() << std::endl;
    }

    if (parser.isSet(optTrack))
    {
        QString tmp = parser.value(optTrack);
        track = tmp.toInt();
        std::cout << "track: " << track << std::endl;
        --track;
    }

    if (parser.isSet(optBiasBegin))
    {
        QString tmp = parser.value(optBiasBegin);
        begin_bias = tmp.toDouble();
        std::cout << "distance begin: " << begin_bias << std::endl;
    }

    if (parser.isSet(optBiasEnd))
    {
        QString tmp = parser.value(optBiasEnd);
        end_bias = tmp.toDouble();
        std::cout << "distance end: " << end_bias << std::endl;
    }

    return CommandLineOK;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::create_directories(const QString &route_dir)
{
    QDir route(route_dir);
    QString topologyDir = route_dir + QDir::separator() + "topology";
    if (!route.exists("topology"))
    {
        route.mkpath(topologyDir);
    }

    trajDir = topologyDir + QString(QDir::separator()) + "trajectories";
    if (!route.exists("trajectories"))
    {
        route.mkpath(trajDir);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string delete_symbol(const std::string &str, char symbol)
{
    std::string tmp = str;
    tmp.erase(std::remove(tmp.begin(), tmp.end(), symbol), tmp.end());
    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
std::string getLine(std::istream &stream)
{
    std::string line = "";
    std::getline(stream, line);
    std::string tmp = delete_symbol(line, '\r');
    tmp = delete_symbol(tmp, ';');
    std::replace(tmp.begin(), tmp.end(), ',', ' ');

    return tmp;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::read_zds_tracks(std::vector<zds_track_t> &zds_tracks,
                                  QString full_path)
{
    std::ifstream in_stream(full_path.toStdString(), std::ios::in);

    if (!in_stream.is_open())
    {
        return;
    }

    std::vector<zds_track_t> tmp_data;
    while (!in_stream.eof())
    {
        std::string line = "";
        line = getLine(in_stream);

        std::istringstream ss(line);

        zds_track_t track;

        ss >> track.begin_point.x
            >> track.begin_point.y
            >> track.begin_point.z
            >> track.end_point.x
            >> track.end_point.y
            >> track.end_point.z
            >> track.prev_uid
            >> track.next_uid
            >> track.arrows
            >> track.voltage
            >> track.ordinate;
        // Если команда в поле arrows пустая, в неё распарситься
        // следующая переменная, проверяем это
        if (((track.arrows == "0") || (track.arrows == "3") || (track.arrows == "25")) &&
            (track.voltage != 0) && (track.voltage != 3) && (track.voltage != 25) &&
            (track.ordinate == 0))
        {
            track.ordinate = track.voltage;
            if (track.arrows == "0")
                track.voltage = 0;
            if (track.arrows == "3")
                track.voltage = 3;
            if (track.arrows == "25")
                track.voltage = 25;
        }
        // У команд n:x.x или m:x.x дробная чать может распарситься
        // как следующая переменная, проверяем это
        if ((track.arrows.front() == 'n' || (track.arrows.front() == 'm')) &&
            (track.voltage != 0) &&
            (track.ordinate == 0 || track.ordinate == 3 || track.ordinate == 25))
        {
            track.voltage = track.ordinate;
            ss >> track.ordinate;
        }
        // Иногда встречается пикетаж со знаком минус
        if (track.ordinate < 0)
        {
            int tmp = abs(track.ordinate);
            if (!tmp_data.empty() && (abs(tmp_data.back().ordinate - tmp) <= 200))
                track.ordinate = tmp;
            else
                track.ordinate = 0;
        }
        // В prev_uid хранится id предыдущего трека, но с нумерацией с 1
        // Можно использовать как индекс данного трека в массиве, где нумерация с 0
        // Только меняем у первого трека prev_uid с -1 на 0
        if (track.prev_uid == -1)
            track.prev_uid =0;

        tmp_data.push_back(track);
    }

    auto it = tmp_data.begin();
    double trajectory_length = 0.0;

    double coord_increase_direction = 1.0;
    double trajectory_recalc_length = 0.0;
    double railway_coord_recalc_begin = 0.0;
    size_t railway_coord_recalc_count = 0;

    bool not_end = true;
    while ( not_end )
    {
        zds_track_t cur_track = *it;
        if (cur_track.next_uid != -2)
        {
            zds_track_t next_track = tmp_data.at(static_cast<size_t>(cur_track.next_uid - 1));
            cur_track.end_point = next_track.begin_point;
            cur_track.railway_coord_end = static_cast<double>(next_track.ordinate);
            *it = next_track;
        }
        else
        {
            not_end = false;
        }

        dvec3 dir_vector = cur_track.end_point - cur_track.begin_point;
        cur_track.length = length(dir_vector);
        cur_track.orth = dir_vector / cur_track.length;
        cur_track.right = normalize(cross(cur_track.orth, dvec3(0.0, 0.0, 1.0)));
        cur_track.up = normalize(cross(cur_track.right, cur_track.orth));

        cur_track.route_coord = trajectory_length;
        trajectory_length += cur_track.length;

        if (not_end)
        {
            // Сохраняем координату в начале трека, если не дробные
            if (railway_coord_recalc_count == 0)
                railway_coord_recalc_begin = static_cast<double>(cur_track.ordinate);

            // Записываем координату в начале трека
            cur_track.railway_coord = railway_coord_recalc_begin + trajectory_recalc_length;

            // В дробных треках ZDS записана одинаковая координата для всех подтреков
            if (abs(railway_coord_recalc_begin - cur_track.railway_coord_end) < 0.01)
            {
                // Считаем количество и суммарную длину подтреков, пересчитываем координату
                railway_coord_recalc_count++;
                trajectory_recalc_length += cur_track.length * coord_increase_direction;
                // Записываем пересчитанную координату конца дробного подтрека
                cur_track.railway_coord_end = railway_coord_recalc_begin + trajectory_recalc_length;
            }
            else
            {
                // Вне дробных подтреков сбрасываем счётчик
                railway_coord_recalc_count = 0;
                trajectory_recalc_length = 0.0;
                // И определяем направление увеличения координаты
                ((cur_track.railway_coord_end - railway_coord_recalc_begin) > 0) ?
                    coord_increase_direction = 1.0 :
                    coord_increase_direction = -1.0;
            }
        }
        else
        {
            if (railway_coord_recalc_count == 0)
                railway_coord_recalc_begin = static_cast<double>(cur_track.ordinate);
            cur_track.railway_coord = railway_coord_recalc_begin + trajectory_recalc_length;
            cur_track.railway_coord_end = cur_track.railway_coord + cur_track.length * coord_increase_direction;
        }

        cur_track.inclination = cur_track.orth.z * 1000.0;
        cur_track.curvature = 0.0;

        zds_tracks.push_back(cur_track);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::generate_topology(const QString &route_dir)
{
    create_directories(route_dir);

    QString full_path = route_dir + QDir::separator() + trkfile;

    std::vector<zds_track_t> zds_tracks;

    read_zds_tracks(zds_tracks, full_path);

    if (zds_tracks.empty())
    {
        return;
    }

    track = cut(track, 0, static_cast<int>(zds_tracks.size()) - 1);

    QString traj_path = trajDir + QDir::separator() +
                        filename + FILE_EXTENTION.c_str();

    QFile file_old(traj_path);
    if (file_old.exists())
        file_old.rename(traj_path + FILE_BACKUP_EXTENTION.c_str());

    QFile file(traj_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    // Расчёт точек сплайна для траектории отклонения
    double trajectory_length = 0.0;
    // Первая точка
    dvec3 mean_right;
    if (track == 0)
        mean_right = zds_tracks[track].right;
    else
        mean_right = normalize(zds_tracks[track].right + zds_tracks[track - 1].right);
    dvec3 point = zds_tracks[track].begin_point + mean_right * begin_bias;
    dvec3 prev_point = point;
    stream                  << point.x
        << DELIMITER_SYMBOL << point.y
        << DELIMITER_SYMBOL << point.z
        << DELIMITER_SYMBOL << static_cast<int>(round(zds_tracks[track].railway_coord))
        << DELIMITER_SYMBOL << trajectory_length
        << "\n";

    // Траектория отклонения
    // Отклонение в ZDS длится 100 метров
    // На всякий случай проверяем разбиение стометрового трека на подтреки
    // Ищем трек, который через 100 м - как трек, который хотя бы через 95 м
    size_t id_end = track;
    double coord_begin = zds_tracks[track].route_coord;
    double railway_coord_begin = zds_tracks[track].railway_coord;
    double main_traj_l = 0.0;
    do
    {
        ++id_end;
        if (id_end < zds_tracks.size())
            main_traj_l = zds_tracks[id_end].route_coord - coord_begin;
        else
        {
            main_traj_l = zds_tracks[id_end - 1].route_coord +
                          zds_tracks[id_end - 1].length - coord_begin;
            break;
        }
    }
    while (main_traj_l < 95.0);
    double railway_coord_length = zds_tracks[id_end].railway_coord - railway_coord_begin;

    // Расчёт промежуточных точек по траектории сплайна отклонения
    for (size_t i = 0; i < NUM_BIAS_POINTS; ++i)
    {
        double coord_add = COORD_COEFF[i] * main_traj_l;
        double coord_ref = coord_begin + coord_add;
        // Находим нужный подтрек - у следующего координата больше
        while ((zds_tracks[track].route_coord < coord_ref) && (track < zds_tracks.size()))
        {
            ++track;
        }

        dvec3 main_track_point = zds_tracks[track].begin_point +
                                 zds_tracks[track].orth * (coord_ref - zds_tracks[track].route_coord);

        // Промежуточная точка отклонения
        double bias_coeff = begin_bias * (1.0 - BIAS_COEFF[i]) +
                            end_bias * BIAS_COEFF[i];
        point = main_track_point + zds_tracks[track].right * bias_coeff;
        trajectory_length += length(point - prev_point);
        prev_point = point;
        stream                  << point.x
            << DELIMITER_SYMBOL << point.y
            << DELIMITER_SYMBOL << point.z
            << DELIMITER_SYMBOL << static_cast<int>(round(railway_coord_begin +
                                                    COORD_COEFF[i] * railway_coord_length))
            << DELIMITER_SYMBOL << trajectory_length
            << "\n";
    }

    // Последняя точка
    double railway_coord;
    if (id_end < zds_tracks.size() - 1)
    {
        mean_right = normalize(zds_tracks[id_end].right + zds_tracks[id_end + 1].right);
        point = zds_tracks[id_end].begin_point + mean_right * end_bias;
        railway_coord = zds_tracks[id_end].railway_coord;
    }
    else
    {
        mean_right = zds_tracks.back().right;
        if (id_end == zds_tracks.size() - 1)
        {
            point = zds_tracks.back().begin_point + mean_right * end_bias;
            railway_coord = zds_tracks.back().railway_coord;
        }
        if (id_end == zds_tracks.size())
        {
            point = zds_tracks.back().end_point + mean_right * end_bias;
            railway_coord = zds_tracks.back().railway_coord_end;
        }
    }
    trajectory_length += length(point - prev_point);
    stream                  << point.x
        << DELIMITER_SYMBOL << point.y
        << DELIMITER_SYMBOL << point.z
        << DELIMITER_SYMBOL << static_cast<int>(round(railway_coord))
        << DELIMITER_SYMBOL << trajectory_length
        << "\n";

    file.close();
}
