#ifndef NEUTRAL_INSERTION_H
#define NEUTRAL_INSERTION_H

struct neutral_insertion_t
{
    float   begin_coord;
    float   end_coord;
    float   length;

    neutral_insertion_t()
        : begin_coord(0.0f)
        , end_coord(0.0f)
        , length(0.0f)
    {

    }
};

#endif // NEUTRAL_INSERTION_H
