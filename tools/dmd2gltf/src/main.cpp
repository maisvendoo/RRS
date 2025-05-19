#include    <Application.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    Application application;

    if (!application.parse_args(argc, argv))
    {
        return -1;
    }

    if (!application.convert())
    {
        return -1;
    }

    return 0;
}
