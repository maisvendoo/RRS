//------------------------------------------------------------------------------
//
//
//
//
//
//------------------------------------------------------------------------------
/*!
 * \file
 * \brief
 * \copyright
 * \author
 * \date
 */


#ifndef     DMD_H
#define     DMD_H

#include    <iostream>
#include    <fstream>

#include    "dmd-mesh.h"

class DMDObject
{
public:

    DMDObject(std::string file_path = "");
    virtual ~DMDObject();

    bool load(const std::string &file_path);

    bool load(std::ifstream &fin);

private:

    size_t line_number;

    std::vector<dmd_mesh_t *> meshes;
};

#endif // DMD_H
