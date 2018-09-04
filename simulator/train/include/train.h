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

#include    "ode-system.h"

#if defined(TRAIN_LIB)
    #define TRAIN_EXPORT    Q_DECL_EXPORT
#else
    #define TRAIN_EXPORT    Q_DECL_IMPORT
#endif

/*!
 * \class
 * \brief
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TRAIN_EXPORT Train : public OdeSystem
{
public:

    /// Constructor
    explicit Train(QObject *parent = Q_NULLPTR);
    /// Destructor
    virtual ~Train();

signals:

    void logMessage(QString msg);

private:



};

#endif // TRAIN_H
