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

    void init() override;

    void step(float t, float dt, const std::vector<float> *server_signals) override;

private:

    enum
    {
        CAB1 = 0,
        CAB2 = 1,
        CABS_NUM
    };

    ControlHandler *revers_handle_holder[CABS_NUM] = {nullptr, nullptr};

    ControlHandler *revers_handle[CABS_NUM] = {nullptr, nullptr};

    ControlHandler *main_handle[CABS_NUM] = {nullptr, nullptr};

    void processKeyboardInput(std::set<uint16_t> &pressed_keys) override;

    void lockReversHandle(int cab_idx);

    void lockMainHandle(int cab_idx);
};

#endif
