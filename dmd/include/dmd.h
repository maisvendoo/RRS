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
#include    "dmd-container.h"

class DMDObject
{
public:

    DMDObject(std::string file_path = "");
    virtual ~DMDObject();

    bool load(const std::string &file_path);

    bool load(std::ifstream &fin);    

private:    

    size_t          line_number;

    dmd_mesh_t      mesh;

    DMDContainer    dmd_container;

    void readNextMesh(DMDContainer &dmd_cont);

    void readTextureBlock(DMDContainer &dmd_cont);

    void calcFaceNormal(const dmd_mesh_t &mesh, face_t &face);
    void calcSmoothNormales(dmd_mesh_t &mesh);
};

#endif // DMD_H
