//------------------------------------------------------------------------------
//
//      Common train's model dynamics
//      (c) maisvendoo, 04/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Common train's model dynamics
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 04/09/2018
 */

#ifndef     TRAIN_H
#define     TRAIN_H

#include    "filesystem.h"
#include    "init_data.h"
#include    "ode-system.h"
#include    "vehicle.h"
#include    "coupling.h"


#if defined(TRAIN_LIB)
    #define TRAIN_EXPORT    Q_DECL_EXPORT
#else
    #define TRAIN_EXPORT    Q_DECL_IMPORT
#endif

/*!
 * \class
 * \brief Common train model
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TRAIN_EXPORT Train : public OdeSystem
{
    Q_OBJECT

public:

    /// Constructor
    explicit Train(FileSystem *fs, QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~Train();

    bool init(const init_data_t &init_data);

signals:

    void logMessage(QString msg);

private:

    FileSystem  *fs;

    double      trainMass;
    double      trainLength;

    /// Order of system ODE motion
    int         ode_order;

    std::vector<Vehicle *> vehicles;

    std::vector<Coupling *> couplings;

    /// Train's loading
    bool loadTrain(QString cfg_path);
    /// Couplings loading
    bool loadCouplings(QString cfg_path);

    /// Set initial conditions
    bool setInitConditions(const init_data_t &init_data);
};

#endif // TRAIN_H
