#ifndef     START_CONFIG_H
#define     START_CONFIG_H

#include    "active-train.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct start_config_t
{
    QString start_config_name = "<Not_selected>";
    std::vector<active_train_t> trains = {};

    start_config_t()
    {

    }
};

#endif // START_CONFIG_H
