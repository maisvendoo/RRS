//------------------------------------------------------------------------------
//
//      Information for loading route by ID which received from TCP
//      (c) maisvendoo, 11/12/2018
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Information for loading route by ID which received from TCP
 * \copyright maisvendoo
 * \author maisvendoo
 * \date 11/12/2018
 */

#ifndef     ROUTE_INFO_H
#define     ROUTE_INFO_H

#include    <string>

/*!
 * \file
 * \brief Route info structure
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct route_info_t
{
    unsigned int    id;
    std::string     name;

    route_info_t()
        : id(0)
        , name("")
    {

    }
};

#endif // ROUTE_INFO_H
