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

    void keysProcess(std::set<uint16_t> &pressed_keys) override;    

    void processMouseInput(io_control_input_t input, uint32_t button, bool is_pressed) override;

    int kmPosBySignal(float signal) const;
};

#endif
