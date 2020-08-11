#include    "traction-controller.h"

#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TractionController::TractionController(QObject *parent) : Device(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TractionController::~TractionController()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TractionController *loadTractionController(QString lib_path)
{
    TractionController *controller = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetTractionController getTractionController = reinterpret_cast<GetTractionController>(lib.resolve("getTractionController"));

        if (getTractionController)
        {
            controller = getTractionController();
        }
    }

    return controller;
}
