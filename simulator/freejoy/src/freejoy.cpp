#include    <freejoy.h>
#include    <Journal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FreeJoy::FreeJoy(QObject *parent) : VirtualInterfaceDevice(parent)
  , joy_id(-1)
  , pos_axisX(0.0)
  , pos_axisY(0.0)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FreeJoy::~FreeJoy()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool FreeJoy::init(QString cfg_path)
{
    // Обновляем статус джойстиков
    sf::Joystick::update();

    // Ищем подключенный джойстик (первый попавшийся, подразумевая FreeJoy)
    while (joy_id < sf::Joystick::Count)
    {
        ++joy_id;

        if (freejoy.isConnected(joy_id))
        {
            Journal::instance()->info(
              "Found control panel " +
              QString(freejoy.getIdentification(joy_id).name.toAnsiString().c_str()));

            break;
        }
    }

    // Не нашли ничего - выходим
    if (joy_id == sf::Joystick::Count)
    {
        Journal::instance()->error("Control panel not found");
        return false;
    }

    // Тут будем читать конфиг


    // Устанавливаем бит готовности к приему сигналов с пульта
    control_signals.analogSignal[FB_READY].setValue(1.0f);

    control_signals.analogSignal[FB_READY].is_active = true;
    control_signals.analogSignal[FB_RBS].is_active = true;
    control_signals.analogSignal[FB_READY].is_active = true;
    control_signals.analogSignal[FB_BRAKE_CRANE].is_active = true;
    control_signals.analogSignal[FB_LOCO_CRANE].is_active = true;

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeJoy::process()
{
    freejoy.update();

    pos_axisX = freejoy.getAxisPosition(joy_id, sf::Joystick::X);
    pos_axisY = freejoy.getAxisPosition(joy_id, sf::Joystick::Y);
    button_pressed = freejoy.isButtonPressed(joy_id, 18);

    control_signals.analogSignal[FB_RBS].setValue(static_cast<float>(button_pressed));

    emit sendControlSignals(control_signals);

    int a = 0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_INTERFACE_DEVICE(FreeJoy)
