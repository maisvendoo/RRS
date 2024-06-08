#ifndef     FREEJOY_H
#define     FREEJOY_H

#define     SFML_STATIC

#include    "virtual-interface-device.h"
#include    "brake-crane-pos.h"

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

    bool RBS_button_pressed;

    bool release_button_pressed;

    double axis_x_min;

    double axis_x_max;

    int RBS_button_id;

    int Release_button_id;

    enum
    {
        BRAKE_CRANE_POS_NUM = 7
    };

    std::array<brake_crane_pos_t, BRAKE_CRANE_POS_NUM> brake_crane_pos;

    sf::Joystick freejoy;

    bool load_config(QString path);
};

#endif
