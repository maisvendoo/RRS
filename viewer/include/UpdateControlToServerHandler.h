#pragma once
#ifndef UPDATE_CONTROL_TO_SERVER_HANDLER_H
#define UPDATE_CONTROL_TO_SERVER_HANDLER_H

#include <vsg/ui/KeyEvent.h>
#include <cstdint>
#include <set>

class TcpClient;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class UpdateControlToServerHandler final : public vsg::Inherit<vsg::Visitor, UpdateControlToServerHandler>
{
public:
    explicit UpdateControlToServerHandler(TcpClient *tc);

    void apply(vsg::KeyPressEvent& keyPress) override;
    void apply(vsg::KeyReleaseEvent& keyRelease) override;
    void apply(vsg::FocusInEvent& focusIn) override;
    void apply(vsg::FocusOutEvent& focusOut) override;
    void changeCurrentVehicle(int current_idx, int controlled_idx);
    void changeCurrentCabine(size_t cabine_idx);

private:

    void sendControlToServer();
    void sendEmptyControlToServer();

    TcpClient *_tcp_client = nullptr;

    uint16_t _current_idx = 0;
    uint16_t _controlled_idx = 0;
    uint32_t _cabine_idx = 0;
    std::set<uint16_t> _pressed_keys = {};
};

#endif // UPDATE_CONTROL_TO_SERVER_HANDLER_H
