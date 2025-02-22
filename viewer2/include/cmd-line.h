#ifndef VIEWER_CMD_LINE_H
#define VIEWER_CMD_LINE_H

#include <optional>
#include <string>

struct cmd_line_t
{
    std::optional<std::string> host_addr;
    std::optional<int> port;
    std::optional<int> width;
    std::optional<int> height;
    bool fullscreen;
    std::optional<std::string> notify_level;
};

#endif // VIEWER_CMD_LINE_H
