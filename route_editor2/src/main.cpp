#include "editor/RouteEditor.h"

#include <cstdlib>

int main()
{
    bool success;
    RouteEditor route_editor(success);
    if (!success)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
