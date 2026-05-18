#include    <Application.h>
#include    <Logger.h>

#include    <algorithm>
#include    <filesystem>
#include    <fstream>
#include    <iostream>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::parse_args(int argc, char* argv[])
{
    cli::Parser parser(argc, argv);

    configure_parser(parser);

    parse_command_line(parser, cmd_line);

    return check_command_line(cmd_line);
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::transform_route()
{
    // Ищем топологию в маршруте
    auto topology_dir = std::filesystem::path(cmd_line.input_route_path.value());
    topology_dir /= "topology";

    if (cmd_line.transform_map.value())
    {
        // Ищем расположение объектов маршрута - папку с файлами *.map
        auto map_dir = std::filesystem::path(topology_dir / "map");
        if (!std::filesystem::exists(map_dir) || !std::filesystem::is_directory(map_dir))
        {
            LOG_WARN("ERROR: map directory does not exists");
            return false;
        }

        // Создаём бэкап имеющихся файлов
        auto backup_dir = std::filesystem::path(topology_dir / "~map");
        if (std::filesystem::exists(backup_dir))
        {
            std::filesystem::remove_all(backup_dir);
        }

        std::filesystem::rename(map_dir, backup_dir);
        std::filesystem::create_directories(map_dir);

        for (const auto& file_it : std::filesystem::directory_iterator(backup_dir))
        {
            if (!file_it.is_regular_file())
            {
                continue;
            }

            auto filename = file_it.path().filename();
            if (filename.extension().string() == ".map")
            {
                std::string old_file_path = (backup_dir / filename).string();
                std::string new_file_path = (map_dir / filename).string();
                translate_map(old_file_path, new_file_path,
                              cmd_line.shift_x.value(),
                              cmd_line.shift_y.value(),
                              cmd_line.shift_z.value());
            }
        }
    }

    if (cmd_line.transform_topology.value())
    {
        // Ищем координаты траекторий маршрута - папку с файлами *.traj
        auto trajectories_dir = std::filesystem::path(topology_dir / "trajectories");
        if (!std::filesystem::exists(trajectories_dir) || !std::filesystem::is_directory(trajectories_dir))
        {
            LOG_WARN("ERROR: trajectories directory does not exists");
            return false;
        }

        // Создаём бэкап имеющихся файлов
        auto backup_dir = std::filesystem::path(topology_dir / "~trajectories");
        if (std::filesystem::exists(backup_dir))
        {
            std::filesystem::remove_all(backup_dir);
        }

        std::filesystem::rename(trajectories_dir, backup_dir);
        std::filesystem::create_directories(trajectories_dir);

        // Ищем траектории путей маршрута - файлы *.traj
        for (const auto& file_it : std::filesystem::directory_iterator(backup_dir))
        {
            if (!file_it.is_regular_file())
            {
                continue;
            }

            auto filename = file_it.path().filename();
            if (filename.extension().string() == ".traj")
            {
                std::string old_file_path = (backup_dir / filename).string();
                std::string new_file_path = (trajectories_dir / filename).string();
                translate_trajectory(old_file_path, new_file_path,
                                     cmd_line.shift_x.value(),
                                     cmd_line.shift_y.value(),
                                     cmd_line.shift_z.value());
            }
        }
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::translate_map(std::string& old_file_path, std::string& new_file_path,
                                double x, double y, double z)
{
    std::ifstream old_file = std::ifstream();
    old_file.open(old_file_path, std::ios::in);
    if (old_file.is_open())
    {
        //LOG_INFO("Info: opened file: %s", old_file_path.c_str());
    }
    else
    {
        LOG_WARN("Warn: failed to open file: %s", old_file_path.c_str());
        return false;
    }

    std::ofstream new_file(new_file_path, std::ios::out);
    if (new_file.is_open())
    {
        //LOG_INFO("Info: opened file: %s", new_file_path.c_str());
        new_file << std::fixed << std::setprecision(6);
    }
    else
    {
        LOG_WARN("Warn: failed to open file: %s", new_file_path.c_str());
        old_file.close();
        return false;
    }

    std::string line_buffer;
    while (std::getline(old_file, line_buffer))
    {
        // Пустое название объекта
        if (line_buffer.empty() || (*(line_buffer.begin()) == ',') )
        {
            new_file << line_buffer << "\n";
            continue;
        }
        // Строка с объектом должна заканчиваться точкой с запятой
        if (*(line_buffer.end() - 1) != ';')
        {
            new_file << line_buffer << "\n";
            continue;
        }
        // Строка с объектом должна содержать шесть запятых - разделителей
        if (std::count(line_buffer.begin(), line_buffer.end(), ',') != 6)
        {
            new_file << line_buffer << "\n";
            continue;
        }

        std::string label = "";
        double pos_x = 0.0;
        double pos_y = 0.0;
        double pos_z = 0.0;
        double rot_x = 0.0;
        double rot_y = 0.0;
        double rot_z = 0.0;

        std::string tmp_buffer = line_buffer;
        std::replace(tmp_buffer.begin(), tmp_buffer.end(), ',', ' ');
        std::istringstream ss(tmp_buffer);
        ss >> label >> pos_x >> pos_y >> pos_z >> rot_x >> rot_y >> rot_z;

        if (ss)
        {
            pos_x += x;
            pos_y += y;
            pos_z += z;
            new_file << label
                << "," << pos_x
                << "," << pos_y
                << "," << pos_z
                << "," << rot_x
                << "," << rot_y
                << "," << rot_z
                << ";\n";
        }
        else
        {
            new_file << line_buffer << "\n";
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::translate_trajectory(std::string& old_file_path, std::string& new_file_path,
                                       double x, double y, double z)
{
    std::ifstream old_file = std::ifstream();
    old_file.open(old_file_path, std::ios::in);
    if (old_file.is_open())
    {
        //LOG_INFO("Info: opened file: %s", old_file_path.c_str());
    }
    else
    {
        LOG_WARN("Warn: failed to open file: %s", old_file_path.c_str());
        return false;
    }

    std::ofstream new_file(new_file_path, std::ios::out);
    if (new_file.is_open())
    {
        //LOG_INFO("Info: opened file: %s", new_file_path.c_str());
        new_file << std::fixed << std::setprecision(6);
    }
    else
    {
        LOG_WARN("Warn: failed to open file: %s", new_file_path.c_str());
        old_file.close();
        return false;
    }

    std::string line_buffer;
    while (std::getline(old_file, line_buffer))
    {
        double pos_x = 0.0;
        double pos_y = 0.0;
        double pos_z = 0.0;
        int railway_coord = 0;
        double len = 0.0;

        std::string tmp_buffer = line_buffer;
        std::istringstream ss(tmp_buffer);
        ss >> pos_x >> pos_y >> pos_z;

        if (ss)
        {
            pos_x += x;
            pos_y += y;
            pos_z += z;
            new_file << pos_x
                << "\t" << pos_y
                << "\t" << pos_z;

            ss >> railway_coord;
            if (ss)
            {
                new_file << "\t" << railway_coord;

                ss >> len;
                if (ss)
                {
                    new_file << "\t" << len;
                }
            }

            new_file << "\n";
        }
        else
        {
            new_file << line_buffer << "\n";
        }
    }
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::configure_parser(cli::Parser &parser)
{
    parser.set_optional<std::string>("i", "input-route",
                                     "",
                                     "Input route path");

    parser.set_optional<double>("x", "delta-x",
                                0.0,
                                "Transform along X axis, meters");

    parser.set_optional<double>("y", "delta-y",
                                0.0,
                                "Transform along Y axis, meters");

    parser.set_optional<double>("z", "delta-z",
                                0.0,
                                "Transform along Z axis, meters");

    parser.set_optional<bool>("m", "map-transform",
                              false,
                              "Transform positions at files /topology/map/*.map");

    parser.set_optional<bool>("t", "topology-transform",
                              false,
                              "Transform positions at files /topology/trajectories/*.traj");

    parser.enable_help();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Application::parse_command_line(cli::Parser &parser, cmd_line_t &cmd_line)
{
    parser.run_and_exit_if_error();
    cmd_line.input_route_path = parser.get<std::string>("i");
    cmd_line.shift_x = parser.get<double>("x");
    cmd_line.shift_y = parser.get<double>("y");
    cmd_line.shift_z = parser.get<double>("z");
    cmd_line.transform_map = parser.get<bool>("m");
    cmd_line.transform_topology = parser.get<bool>("t");
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Application::check_command_line(const cmd_line_t &cmd_line)
{
    if (!cmd_line.input_route_path.has_value())
    {
        LOG_WARN("ERROR: Missing input route path");
        return false;
    }

    if (!std::filesystem::exists(cmd_line.input_route_path.value()))
    {
        LOG_WARN("ERROR: input route path not exists");
        return false;
    }

    if (!cmd_line.transform_map.value() && !cmd_line.transform_topology.value())
    {
        LOG_WARN("ERROR: Nothing to do, options --map-transform or --topology-transform are missing");
        return false;
    }

    if ((cmd_line.shift_x.value() == 0.0) && (cmd_line.shift_y.value() == 0.0) && (cmd_line.shift_z.value() == 0.0))
    {
        LOG_WARN("ERROR: Nothing to do, --delta-x, --delta-y and --delta-z are all missing or set to zero");
        return false;
    }

    return true;
}
