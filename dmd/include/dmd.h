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

    dmd_multymesh_t getMultyMesh() const;

private:

    size_t line_number;

    dmd_multymesh_t multyMesh;

    void readNextMesh(std::ifstream &fin);

    void readTextureBlock(std::ifstream &fin);

    void calcNormales(dmd_mesh_t *mesh);
    void calcSmoothNormales(dmd_mesh_t *mesh);
};

#endif // DMD_H
