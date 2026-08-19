#ifndef     TRAJECTORY_H
#define     TRAJECTORY_H

#include    <QObject>
#include    <QMap>
#include    <QSet>

#include    "CfgReader.h"
#include    "topology-export.h"
#include    "topology-defines.h"
#include    "track.h"

#include    <rail-profile.h>

#include    <profile-point.h>

class Signal;
class TrajectoryDevice;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct module_cfg_t
{
    CfgReader cfg;
    QString module_name = "";
    QSet<QString> traj_names = {};
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class TOPOLOGY_EXPORT Trajectory : public QObject
{
    Q_OBJECT

public:

    Trajectory(QObject *parent = nullptr);

    ~Trajectory();

    bool load(const QString &route_dir, const QString &traj_name,
              std::vector<module_cfg_t>& modules, bool solve_errors = true);

    QString getName() const;

    double getLength() const;

    void setFwdSwitch(Switch* switch_ptr);
    void setBwdSwitch(Switch* switch_ptr);

    Switch* getNextSwitch(dir_t& dir) const;

    /// Задать включение траектории в маршрут ДЦ
    void setInRoute(bool is_route);

    /// Включение траектории в маршрут ДЦ
    bool isInRoute() const;

    /// Задать занятость единицей подвижного состава idx в интервале координат
    void setBusy(size_t idx, double coord_begin, double coord_end);

    /// Очистить занятость подвижным составом
    void clearBusy();

    /// Задать признак занятости (для работы копии топологии вне движка)
    void setBusyState(bool busy_state);

    /// Признак занятости подвижным составом
    bool isBusy() const;

    /// Проверка занятости подвижным составом в интервале координат
    bool isBusy(double coord_begin, double coord_end) const;

    /// Индекс ближайшей единицы подвижного состава, если есть;
    /// -1, если нет подвижного состава в пределах дистанции поиска
    int getBusyVehicle(double &distance, double coord, double search_distance, dir_t direction) const;

    /// Интервал координат, занятых подвижным составом;
    /// если пустая, busy_begin_coord = length; busy_end_coord = 0.0
    void getBusyCoords(double &busy_begin_coord, double &busy_end_coord) const;

    /// Вернуть все треки траектории
    const std::vector<track_t>& getTracks() const;

    /// Вернуть первый трек траектории
    const track_t& getFirstTrack() const;

    /// Вернуть последний трек траектории
    const track_t& getLastTrack() const;

    /// Получить оборудование путевой инфраструктуры на этой траектории
    const std::vector<TrajectoryDevice*>& getTrajectoryDevices() const;

    /// Светофор вперёд, включающий данную траекторию в маршрут ДЦ
    Signal* getRouteBySignalFwd() const;
    void setRouteBySignalFwd(Signal* signal);

    /// Светофор назад, включающий данную траекторию в маршрут ДЦ
    Signal* getRouteBySignalBwd() const;
    void setRouteBySignalBwd(Signal* signal);

    /// Шаг симуляции
    virtual void step(double t, double dt);

    QByteArray serialize() const;

    void deserialize(QByteArray &data);

    /// Поиск новой траектории, траекторной координаты и смены ориентации,
    /// возвращает false, если координата за пределы топологии (за тупик)
    static bool findTrajectoryAtCoord(Trajectory*& cur_traj, double& coord, dir_t& orient);
    static bool findTrajectoryAtCoord(Trajectory*& cur_traj, double& coord, double& coord_off, dir_t& orient);

    /// Получить положение ПЕ на траектории
    profile_point_t getPosition(double traj_coord, int direction) const;

    /// Вертикальное смещение рельса от неровностей пути в точке
    /// traj_coord, м. side: 0 - левый рельс, 1 - правый.
    /// Профиль детерминирован: одинаков для всех ПЕ и повторных проездов
    double getRailHeight(double traj_coord, int side) const;

    /// Возвышение наружного рельса в точке traj_coord, мм (Б16).
    /// Линейная интерполяция между cant_mm соседних треков: переходный
    /// отвод возвышения растянут на длину граничного трека.
    /// Знак: "+" - левый рельс выше, "-" - правый
    double getCant(double traj_coord) const;

    /// Боковое смещение оси пути от неровностей плана линии, м
    double getLateralOffset(double traj_coord) const;

    /// Повреждение пути сходом ПЕ (ТЗ "Реалистичный сход ПС", п.27):
    /// масштаб неровностей растёт, влияя на следующие поезда
    void damageTrack(double factor);

    /// Накопление тоннажа на путь от прохода ПЕ (ТЗ "43-47", износ
    /// рельсов): mass_tonnes - масса ПЕ, т; distance_m - пройденный путь, м
    void addTonnage(double traj_coord, double mass_tonnes, double distance_m);

signals:

    void sendTrajBusyState(QByteArray busy_data);

    /// Сигнал для модели, сообщающий индекс ПЕ, занявшей траекторию
    void sigTrajChangeState(int vehicle_idx, bool is_busy, QString traj_name);

private:

    QString name = "";

    double len = 0.0;

    /// Индекс последней ПЕ занимавшей данную траекторию
    int last_bused_index = 0;

    /// признак занятости траектории
    bool is_busy = false;

    bool prev_is_busy = false;

    /// признак включения траектории в маршрут ДЦ
    bool in_route = false;

    bool prev_in_route = false;

    /// Светофор вперёд, включающий данную траекторию в маршрут ДЦ
    Signal* in_route_by_signal_fwd = nullptr;
    /// Светофор назад, включающий данную траекторию в маршрут ДЦ
    Signal* in_route_by_signal_bwd = nullptr;

    QMap<size_t, std::array<double, 2>> vehicles_coords;

    Switch* fwd_switch = nullptr;

    Switch* bwd_switch = nullptr;

    std::vector<track_t> tracks;

    /// Профиль вертикальных неровностей пути данной траектории
    track::RailProfile rail_profile;

    /// Построение профиля неровностей пути (конфиг маршрута кэшируется)
    void loadRailProfile(const QString& route_dir);

    /// Неровность стрелочного перевода уже поставлена у конца/начала
    /// траектории (защита от повторной генерации при перепривязке стрелок)
    bool switch_irreg_fwd = false;
    bool switch_irreg_bwd = false;

    /// Автогенерация неровности крестовины у конца (fwd=true) или начала
    /// траектории, подключённого к стрелке (ТЗ "Неровности", п.15)
    void generateSwitchIrregularity(bool at_fwd_end);

    /// Оборудование путевой инфраструктуры на этой траектории
    std::vector<TrajectoryDevice *> devices;

    /// Поиск текущего и следующего трека
    void findTracks(double traj_coord,
                    track_t &cur_track,
                    track_t &prev_track,
                    track_t &next_track) const;

    /// Поиск трека на следующей траектории
    track_t findNextTrack(const track_t& cur_track, dir_t dir) const;

    /// Создание условного продолжения топологии за тупик для корректного расчёта
    track_t createFakeTrack(const track_t& cur_track, dir_t dir) const;

    /// Создание трека в обратном направлении для корректного расчёта
    track_t createReversedTrack(const track_t& track) const;

    /// Расчёт кривизны между двумя соседними треками
    double calc_curvature(const track_t& track0, const track_t& track1) const;
};

#endif
