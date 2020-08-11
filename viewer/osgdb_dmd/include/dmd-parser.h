#ifndef     DMD_PARSER_H
#define     DMD_PARSER_H

#include    "dmd-mesh.h"
#include    <fstream>

dmd_mesh_t load_dmd(std::ifstream &stream);

void readNextMesh(std::ifstream &stream, dmd_mesh_t &mesh);

void readTextureBlock(std::ifstream &stream, dmd_mesh_t &mesh);

#endif // DMD_PARSER_H
