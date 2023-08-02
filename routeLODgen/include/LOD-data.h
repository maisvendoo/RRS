#ifndef     LOD_DATA_H
#define     LOD_DATA_H

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct LOD_data_t
{
    size_t level;
    double reduction;
    double min_dist;
    double max_dist;

    LOD_data_t()
        : reduction(1.0)
        , min_dist(0.0)
        , max_dist(1000.0)
    {

    }

    QString getNameSuphyx() const
    {
        return QString("_LOD%1_%2_%3")
                .arg(level)
                .arg(static_cast<int>(min_dist))
                .arg(static_cast<int>(max_dist));
    }
};

#endif // LOD_DATA_H
