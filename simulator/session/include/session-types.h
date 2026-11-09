//------------------------------------------------------------------------------
//
//      Снимок состояния многопользовательской сессии (ТЗ "RP-сервер", п.5)
//
//------------------------------------------------------------------------------

#ifndef SESSION_TYPES_H
#define SESSION_TYPES_H

#include    <QString>
#include    <QStringList>

#include    <cstdint>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
namespace session
{

//------------------------------------------------------------------------------
/// Состояние одной подвижной единицы в сейве
//------------------------------------------------------------------------------
struct vehicle_state_t
{
    int model_index = -1;
    double railway_coord = 0.0;
    double velocity_kmh = 0.0;
    QString config_name = "";

    /// Уравнительный резервуар, кгс/см²
    double er_pressure = 0.0;
    bool pantograph_up = false;

    QString traj_name = "";
    double traj_coord = 0.0;
    int traj_dir = 0;
};

//------------------------------------------------------------------------------
/// Состояние поезда (головной ПЕ задаёт позицию)
//------------------------------------------------------------------------------
struct train_state_t
{
    int train_idx = -1;
    QString name = "";
    int tab_number = -1;
    QString train_config = "";

    QString trajectory_name = "";
    double init_coord = 0.0;
    int direction = 0;
    double init_velocity = 0.0;

    std::vector<vehicle_state_t> vehicles;
};

//------------------------------------------------------------------------------
/// Состояние стрелки
//------------------------------------------------------------------------------
struct switch_state_t
{
    QString name = "";
    int state_fwd = 0;
    int state_bwd = 0;
    int ref_state_fwd = 0;
    int ref_state_bwd = 0;
};

//------------------------------------------------------------------------------
/// Состояние клиента (подключён или "зависший", ТЗ п.6, 9)
//------------------------------------------------------------------------------
struct client_state_t
{
    int client_id = -1;
    int tab_number = -1;
    int vehicle_idx = -1;
    int cab_idx = -1;
    bool connected = false;
};

//------------------------------------------------------------------------------
/// Полный снимок сессии
//------------------------------------------------------------------------------
struct session_state_t
{
    std::uint32_t date_data = 0;
    std::uint32_t time_data = 0;
    double simulation_seconds = 0.0;
    QString time_string = "";

    QString route_name = "";

    std::vector<train_state_t> trains;
    std::vector<switch_state_t> switches;
    std::vector<client_state_t> clients;
};

} // namespace session

#endif // SESSION_TYPES_H
