#ifndef     TRAIN_INFO_H
#define     TRAIN_INFO_H

#include    <QString>

struct train_info_t
{
    QString train_config_path;
    QString train_title;
    QString description;

    train_info_t()
        : train_config_path("")
        , train_title("")
        , description("")
    {

    }
};

#endif // TRAININFO_H
