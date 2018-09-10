//------------------------------------------------------------------------------
//
//      Data types for working with railway profile
//      (c) maisvendoo, 09/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Data types for working with railway profile
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 09/09/2018
 */

#ifndef     PROFILE_ELEMENT_H
#define     PROFILE_ELEMENT_H

#include    <vector>

/*!
 * \struct
 * \brief Profile element
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct profile_element_t
{
    double  railway_coord;
    double  inclination;
    double  curvature;

    profile_element_t()
        : railway_coord(0.0)
        , inclination(0.0)
        , curvature(0.0)
    {

    }
};

/*!
 * \typedef
 * \brief Profile container
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::vector<profile_element_t *>    profile_t;

#endif // PROFILEELEMENT_H
