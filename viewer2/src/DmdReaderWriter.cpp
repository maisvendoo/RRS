#include "DmdReaderWriter.h"
#include "Logger.h"
#include <cstdint>
#include <fstream>
#include <string>

DmdReaderWriter::DmdReaderWriter()
{

}

DmdReaderWriter::~DmdReaderWriter()
{

}

vsg::ref_ptr<vsg::Object> DmdReaderWriter::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
{
    auto extension = filename.substr(filename.find_last_of('.'));
    if (extension != ".dmdu")
    {
        return {};
    }

    std::ifstream united_file(filename);
    if (!united_file)
    {
        LOG_ERROR("United file %s does not open", filename.c_str());
        return {};
    }

    std::string model_path;
    std::string texture_path;
    std::getline(united_file, model_path);
    std::getline(united_file, texture_path);
    united_file.close();

    std::ifstream model_file(model_path);
    if (!model_file)
    {
        LOG_ERROR("Model file %s does not open", model_path.c_str());
        return {};
    }

    std::string buffer = "";
    while (buffer != "TriMesh()")
    {
        model_file >> buffer;
        if (model_file.eof())
        {
            LOG_ERROR("No TriMesh() line in model file %s", model_path.c_str());
            return {};
        }
    }

    model_file >> buffer >> buffer;

    std::uint32_t numverts, numfaces;
    model_file >> numverts >> numfaces;

    LOG_INFO("%d %d", numverts, numfaces);
    return {};
}
