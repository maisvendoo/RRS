//------------------------------------------------------------------------------
//
//      Global application constants
//      (c) maisvendoo, 02/09/2018
//      Developer: Dmitry Pritykin
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Global application constants
 * \copyright maisvendoo
 * \author Dmitry Pritykin
 * \date 02/09/2018
 */

#ifndef     GLOBAL_CONST_H
#define     GLOBAL_CONST_H

#define     APPLICATION_NAME    QString("simulator")
#define     APPLICATION_VERSION QString("0.1.0")

#define     SHARED_MEMORY_SIM_INFO      QString("siminfo")
#define     SHARED_MEMORY_SIM_UPDATE    QString("simupdate")
#define     SHARED_MEMORY_KEYS_DATA     QString("keys")
#define     SHARED_MEMORY_CONTROLLED    QString("controlled")

#define     MAX_NUM_VEHICLES      180

#define     DEBUG_STRING_SIZE           512
#define     KEYS_DATA_BYTEARRAY_SIZE    2000

#define     ROUTE_DIR_NAME_SIZE             256
#define     VEHICLE_CONFIG_DIR_NAME_SIZE    256
#define     VEHICLE_CONFIG_FILENAME_SIZE    256

#endif // GLOBAL_CONST_H
