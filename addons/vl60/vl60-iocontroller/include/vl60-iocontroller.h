#ifndef     VL60_IO_CONTROLLER_H
#define     VL60_IO_CONTROLLER_H

#include    <io-controller.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class VL60IOController : public IOController
{
public:

    VL60IOController();

    ~VL60IOController() = default;

private:

    void processKeyboardInput(std::set<uint16_t> &pressed_keys) override;
};

#endif
