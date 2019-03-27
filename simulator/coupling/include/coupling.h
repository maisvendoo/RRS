//------------------------------------------------------------------------------
//
//      Abstract coupling model
//      (c) maisvendoo 04/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Abstract coupling model
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 04/09/2018
 */

#ifndef     COUPLING_H
#define     COUPLING_H

#include    <QtGlobal>
#include    <QString>

#if defined(COUPLING_LIB)
    #define COUPLING_EXPORT Q_DECL_EXPORT
#else
    #define COUPLING_EXPORT Q_DECL_IMPORT
#endif

/*!
 * \class
 * \brief Abstract coupling class
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class COUPLING_EXPORT Coupling
{
public:

    /// Constructor
    Coupling();
    /// Destructor
    virtual ~Coupling();

    /// Get coupling force
    virtual double getForce(double ds, double dv) = 0;

    /// Reset coupling (reset getForce() calls count)
    virtual void reset();

    /// Load configuration
    void loadConfiguration(QString cfg_path);

protected:

    /// Count of calls getForce()
    int     calls_count;

    /// Gap in coupling mechanism
    double  delta;

    /// Maximal motion range
    double lambda;

    /// Construct shiffness
    double  ck;

    /// Friction coeff
    double  beta;

    ///
    virtual void loadConfig(QString cfg_path);
};

/*!
 * \typedef
 * \brief getCoupling() signature
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef Coupling* (*GetCoupling)();

/*!
 * \def
 * \brief Macro for getCoupling() generation
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_COUPLING(ClassName) \
    extern "C" Q_DECL_EXPORT Coupling *getCoupling() \
    {\
        return new (ClassName)(); \
    }

/*!
 * \fn
 * \brief Load coupling from library
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT Coupling *loadCoupling(QString lib_path);

#endif // COUPLING_H
