#ifndef     APPLICATION_H
#define     APPLICATION_H

#include    <string>
#include    <cmdparser.hpp>
#include    <command-line.h>

struct Geometry;

using std::string_literals::operator""s;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Application
{
    enum ConvertMode
    {
        CONVERT_ROUTE,
        CONVERT_MODEL
    };

public:

    /// Разбор аругментов командной строки
    bool parse_args(int argc, char* argv[]);

    /// Конвертация
    bool convert();

private:

    /// Конвертация маршрута целиком
    bool convert_route(std::string &in_dmd_route_path,
                       std::string &out_gltf_route_path,
                       bool only_used_at_map);

    /// Конвертация отдельной модели
    bool convert_model(std::string &in_dmd_model_path,
                       std::string &in_texture_path,
                       std::string &out_gltf_model_path,
                       std::string out_relative_bin_path = "",
                       std::string out_relative_texture_path = "");

    /// Получение данных о модели из DMD-файла
    bool get_dmd_model_data(std::string &in_dmd_model_path, Geometry& model_data);

    /// Генерация GLTF-модели
    bool generate_gltf_model(Geometry& model_data,
                             std::string &in_texture_path,
                             std::string &gltf_directory_path,
                             std::string &out_relative_bin_path,
                             std::string &out_relative_texture_path);

private:

    ConvertMode convert_mode;

    cmd_line_t cmd_line;

    /// Настройка парсера командной строки
    void configure_parser(cli::Parser &parser);

    /// Разбор командной строки
    void parse_command_line(cli::Parser &parser, cmd_line_t &cmd_line);

    /// Выбор режима конвертации
    bool set_convert_mode(const cmd_line_t &cmd_line, ConvertMode &convert_mode);
};

#endif // APPLICATION_H
