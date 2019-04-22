#include    "main.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    Application *app = new Application(argc, argv);

    return app->exec();
}
