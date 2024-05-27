#include    <freejoy.h>
#include    <Journal.h>
#include    <CfgReader.h>

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
FreeJoy::FreeJoy(QObject *parent) : VirtualInterfaceDevice(parent)
  , joy_id(-1)
  , pos_axisX(0.0)
  , pos_axisY(0.0)
  , RBS_button_pressed(false)
  , axis_x_min(0)
  , axis_x_max(1)
  , RBS_button_id(18)
  , Release_button_id(16)
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
    if (!load_config(cfg_path + QDir::separator() + "freejoy.xml"))
    {
        Journal::instance()->error("FreeJoy config not found");
        return false;
    }

    // Устанавливаем бит готовности к приему сигналов с пульта
    control_signals.analogSignal[FB_READY].setValue(1.0f);    

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void FreeJoy::process()
{
    freejoy.update();

    pos_axisX = static_cast<int>(freejoy.getAxisPosition(joy_id, sf::Joystick::X));
    pos_axisY = static_cast<int>(freejoy.getAxisPosition(joy_id, sf::Joystick::Y));
    RBS_button_pressed = freejoy.isButtonPressed(joy_id, RBS_button_id);
    release_button_pressed = freejoy.isButtonPressed(joy_id, Release_button_id);

    control_signals.analogSignal[FB_RBS].setValue(static_cast<float>(RBS_button_pressed));
    control_signals.analogSignal[FB_LOCO_CRANE].setValue( (pos_axisX - axis_x_min) / (axis_x_max - axis_x_min));

    for (size_t i = 0; i < brake_crane_pos.size(); ++i)
    {
        if ( (pos_axisY >= brake_crane_pos[i].axis_y_min) &&
             (pos_axisY < brake_crane_pos[i].axis_y_max) )
        {
            control_signals.analogSignal[FB_BRAKE_CRANE].setValue(i);
            break;
        }
    }

    control_signals.analogSignal[FB_RELEASE_VALVE].setValue(static_cast<float>(release_button_pressed));

    emit sendControlSignals(control_signals);    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool FreeJoy::load_config(QString path)
{
    CfgReader cfg;

    if (!cfg.load(path))
    {
        return false;
    }

    QString locoCraneSec = "LocoCrane";

    cfg.getDouble(locoCraneSec, "AxisXmin", axis_x_min);
    cfg.getDouble(locoCraneSec, "AxisXmax", axis_x_max);

    QDomNode brakeCranePosSec = cfg.getFirstSection("BrakeCranePos");

    while (!brakeCranePosSec.isNull())
    {
        brake_crane_pos_t brake_pos;

        cfg.getInt(brakeCranePosSec, "Pos", brake_pos.pos_num);
        cfg.getDouble(brakeCranePosSec, "AxisYmin", brake_pos.axis_y_min);
        cfg.getDouble(brakeCranePosSec, "AxisYmax", brake_pos.axis_y_max);

        brake_crane_pos[brake_pos.pos_num] = brake_pos;

        brakeCranePosSec = cfg.getNextSection();
    }

    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
GET_INTERFACE_DEVICE(FreeJoy)
