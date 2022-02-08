#ifndef     ALSN_STRUCT_H
#define     ALSN_STRUCT_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
enum
{
    ALSN_WHITE = 0,
    ALSN_RED = 1,
    ALSN_RED_YELLOW = 2,
    ALSN_YELLOW = 3,
    ALSN_GREEN = 4
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#pragma pack(push, 1)

struct alsn_info_t
{
    short   code_alsn;
    int     num_free_block;
    int     response_code;
    char    current_time[9];
    char    signal_name[23];
    double  signal_dist;
    int     power_type;
    float   voltage;


    alsn_info_t()
        : code_alsn(ALSN_GREEN)
        , num_free_block(1)
        , response_code(0)
        , current_time("00:00:00")
        , signal_name("")
        , signal_dist(0.0)
        , power_type(0)
        , voltage(0.0)
    {

    }
};

#pragma pack(pop)

#endif // ALSNSTRUCT_H
