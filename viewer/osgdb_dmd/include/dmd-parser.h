#ifndef     DMD_PARSER_H
#define     DMD_PARSER_H

#include    "dmd-mesh.h"
#include    <fstream>

dmd_multimesh_t load_dmd(std::ifstream &stream);

void readNextMesh(std::ifstream &stream, dmd_multimesh_t &multimesh);

void readTextureBlock(std::ifstream &stream, dmd_multimesh_t &multimesh);

#endif // DMD_PARSER_H
