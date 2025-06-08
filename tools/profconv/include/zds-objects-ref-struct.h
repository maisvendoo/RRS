#ifndef     ZDS_OBJECTS_REF_H
#define     ZDS_OBJECTS_REF_H

#include    <string>
#include    <map>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct zds_object_ref_t
{
    std::string object_name = "";
    std::string model_path = "";
    std::string texture_path = "";
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef std::map<std::string, zds_object_ref_t *> zds_objects_ref_data_t;

#endif // ZDS_OBJECTS_REF_H
