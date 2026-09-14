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

    void processMouseControl(io_control_input_t &io_ctrl, int button) override;

    /// Тумблеры "включено/выключено" (Shift+клавиша / Ctrl+клавиша)
    void processTumblers(const std::set<uint16_t> &pressed_keys);

    /// Расшифровка сигнала КМЭ (53) в позицию 0..33
    int kmPosBySignal(float signal) const;
};

#endif
