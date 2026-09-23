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

    QString getControlStateText(const io_control_input_t &io_ctrl,
                                float state) const override;

    /// ╨в╤Г╨╝╨▒╨╗╨╡╤А╤Л "╨▓╨║╨╗╤О╤З╨╡╨╜╨╛/╨▓╤Л╨║╨╗╤О╤З╨╡╨╜╨╛" (Shift+╨║╨╗╨░╨▓╨╕╤И╨░ / Ctrl+╨║╨╗╨░╨▓╨╕╤И╨░)
    void processTumblers(const std::set<uint16_t> &pressed_keys);

    /// ╨а╨░╤Б╤И╨╕╤Д╤А╨╛╨▓╨║╨░ ╤Б╨╕╨│╨╜╨░╨╗╨░ ╨Ъ╨Ь╨н (53) ╨▓ ╨┐╨╛╨╖╨╕╤Ж╨╕╤О 0..33
    int kmPosBySignal(float signal) const;
};

#endif
