//------------------------------------------------------------------------------
//
//      Global environment lighting (Sun emulation)
//      (c) maisvendoo
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief Global environment lighting (Sun emulation)
 * \copyright maisvendoo
 * \author maisvendoo
 * \date
 */

#ifndef		LIGHTING_H
#define		LIGHTING_H

#include    <osg/Light>
#include    <osg/LightSource>

/*!
 * \fn
 * \brief "Sun" initialization
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void initEnvironmentLight(osg::Group *root,
                          osg::Vec4 color,
                          float power,
                          float psi,
                          float theta);

#endif
