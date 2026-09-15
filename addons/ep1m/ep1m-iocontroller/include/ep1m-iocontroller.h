#ifndef     EP1M_IO_CONTROLLER_H
#define     EP1M_IO_CONTROLLER_H

#include    <io-controller.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class EP1MIOController : public IOController
{
public:

    EP1MIOController();

    ~EP1MIOController() = default;

private:

    void keysProcess(std::set<uint16_t> &pressed_keys) override;

    void processMouseControl(io_control_input_t &io_ctrl, int button) override;

    /// Тумблеры с клавишами включения/отключения (Shift+клавиша / Ctrl+клавиша)
    void processTumblers(const std::set<uint16_t> &pressed_keys);
};

#endif
