#ifndef     FREEJOY_H
#define     FREEJOY_H

#define     SFML_STATIC

#include    "virtual-interface-device.h"

#include    <SFML/Window/Joystick.hpp>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class FreeJoy : public VirtualInterfaceDevice
{
public:

    FreeJoy(QObject *parent = Q_NULLPTR);

    ~FreeJoy();

    bool init(QString cfg_path);

    void process();

private:

    int joy_id;

    double pos_axisX;

    double pos_axisY;

    bool button_pressed;

    sf::Joystick freejoy;
};

#endif
