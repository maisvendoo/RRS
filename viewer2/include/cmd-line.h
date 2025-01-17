#ifndef VIEWER_CMD_LINE_H
#define VIEWER_CMD_LINE_H

#include <optional>
#include <string>

struct cmd_line_t
{
    std::optional<std::string> route_dir;
    std::optional<std::string> train_config;
    std::optional<std::string> host_addr;
    std::optional<int> port;
    std::optional<int> width;
    std::optional<int> height;
    bool fullscreen;
    bool localmode;
    std::optional<std::string> notify_level;
    std::optional<int> direction;
};

#endif // VIEWER_CMD_LINE_H
