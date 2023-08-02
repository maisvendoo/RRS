#ifndef     OBJECT_DATA_H
#define     OBJECT_DATA_H

#include    <QString>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct object_data_t
{
    QString model_ID;
    QString model_path;

    std::vector<QString> texture_path;

    object_data_t()
        : model_ID("")
        , model_path("")

    {
        std::fill(texture_path.begin(), texture_path.end(), "");
    }
};

#endif // OBJECT_DATA_H
