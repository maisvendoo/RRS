#ifndef MASK_H
#define MASK_H

enum Mask
{
    // Renders first
    MASK_SCENE = 0x01,
    // Renders after MASK_SCENE
    MASK_GUI1 = 0x02,
    // Renders after MASK_GUI1
    MASK_GUI2 = 0x04,
    // Reacts on intersections
    MASK_CLICKABLE = 0x08
};

#endif // MASK_H
