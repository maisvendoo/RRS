#ifndef     PNEUMO_HOSE_H
#define     PNEUMO_HOSE_H

#include    "device.h"
#include    "vec3.h"

#include    "pneumo-hose-data.h"

//------------------------------------------------------------------------------
// Рукав пневматической магистрали
//------------------------------------------------------------------------------
class DEVICE_EXPORT PneumoHose : public Device
{
public:

    /// Конструктор
    PneumoHose(QObject *parent = nullptr);

    /// Деструктор
    ~PneumoHose();

    /// Legacy
    void setKeySymbolConnect(std::uint16_t key_symbol);

    /// Legacy
    void setKeyModifierConnect(std::uint16_t key_modifier);

    /// Legacy
    void setKeySymbolDisconnect(std::uint16_t key_symbol);

    /// Legacy
    void setKeyModifierDisconnect(std::uint16_t key_modifier);

    /// Legacy
    void setControl(std::set<uint16_t>* keys = nullptr,
                    control_signals_t* control_signals = nullptr) override;

    /// Соединить рукава
    void connect();

    /// Разъединить рукава
    void disconnect();

    /// Закрепить рукав на подвесе
    void fix();

    /// Снять рукав с подвеса
    void unfix();

    /// Установить состояние рукава через IOController (0 — разъединить, 1 — нейтраль, 2 — соединить)
    void setExternalState(int command);

    /// Состояние соединения рукавов
    bool isConnected() const;

    /// Закрепление рукава на подвесе
    bool isFixed() const;

    /// Задать давление в рукаве
    void setPressure(double value);

    /// Задать коэффициент перетока через рукав
    void setFlowCoeff(double value);

    /// Задать длину средней части рукава (от точки изгиба в начале до точки изгиба перед соединительной головкой), м
    void setMiddleLength(double value);

    /// Задать длину жёсткого окончания рукава с соединительной головкой, м
    void setHeadLength(double value);

    /// Задать точку изгиба в начале рукава, в локальных координатах подвижной единицы, м
    void setRootPosition(dvec3 position);

    /// Задать углы отклонения рукава от продольной оси
    /// (конструктивный, в свободном и в закреплённом на подвесе состоянии), радиан
    void setSideAngles(double existing, double free, double fixed);

    /// Задать углы свешивания рукава вниз от продольной оси
    /// (конструктивный, в свободном и в закреплённом на подвесе состоянии), радиан
    void setDownAngles(double existing, double free, double fixed);

    /// Задать углы поворота соединительной головки вверх
    /// (конструктивный, в свободном и в закреплённом на подвесе состоянии, для соединения/разъединения), радиан
    void setHeadUpAngles(double existing, double free, double fixed, double ready_to_connect = 36.0 / 180.0 * Physics::PI);

    /// Задать положение подвижной единицы и векторы её локальных координат, м
    void setVehiclePosition(const dvec3 position,
                            const dvec3 vector_right,
                            const dvec3 vector_orth,
                            const dvec3 vector_up);

    /// Получить поток через рукав
    double getFlow() const;

    /// Получить угол отклонения рукава от продольной оси
    /// (<0 - повёрнут влево, >0 - повёрнут вправо),
    /// за вычетом конструктивного, для анимации, радиан
    double getSideAngle() const;

    /// Получить угол свешивания рукава вниз от продольной оси,
    /// за вычетом конструктивного, для анимации, радиан
    double getDownAngle() const;

    /// Получить угол поворота соединительной головки вверх,
    /// за вычетом конструктивного, для анимации, радиан
    double getHeadUpAngle() const;

    dvec3 getConnectionPoint() const;

    virtual void preStep(state_vector_t& Y, double t) override;

    enum {
        PIPE_DRAIN_FLOW_SOUND = 0,  ///< Звук опорожнения магистрали через рукав
        CONNECT_SOUND,      ///< Звук соединения рукавов
        DISCONNECT_SOUND,   ///< Звук рассоединения рукавов
        NUM_SOUNDS,
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = PIPE_DRAIN_FLOW_SOUND) const override;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = PIPE_DRAIN_FLOW_SOUND) const override;

protected:

    /// Legacy
    std::uint16_t key_symbol_inc = KEY_Undefined;
    /// Legacy
    std::uint16_t key_modifier_inc = KEY_Undefined;
    /// Legacy
    std::uint16_t key_symbol_dec = KEY_Undefined;
    /// Legacy
    std::uint16_t key_modifier_dec = KEY_Undefined;

    /// Коэффициент громкости озвучки к потоку опорожнения магистрали через рукав
    double K_sound = 1.5;

    /// Звук опорожнения магистрали через рукав
    sound_state_t atm_flow_sound = sound_state_t();

    enum {
        SIDE_ANGLE = 0,
        DOWN_ANGLE,
        HEAD_UP_ANGLE,
        DOWN_ANGLE_VELOCITY,
        HEAD_UP_ANGLE_ANGLE_VELOCITY,
        NUM_ANGLES
    };

    enum State {
        DO_FIX = 0, ///< Рукав выполняет поворот к подвесу
        FIXED,      ///< Рукав закреплён на подвесе
        FREE,       ///< Рукав свободно висит
        CONNECTED,  ///< Рукав соединён с соседним
        DO_CONNECT  ///< Рукав выполняет поворот для соединения с соседним
    } state = FREE;

    /// Команда разъединить рукава / закрепить рукав на подвесе
    bool command_disconnect_fix = false;
    bool prev_command_disconnect_fix = false;

    /// Команда снять рукав с подвеса / соединить рукава
    bool command_unfix_connect = false;
    bool prev_command_unfix_connect = false;

    /// Вектор от точки крепления рукава к точке соединения с соседним рукавом
    // Сохраняем значения для отладки, в релизе можно будет убрать
    dvec3 v_hose = {0.0, 0.0, 0.0};

    /// Вектор поперечной оси подвижной единицы
    dvec3 v_right = {1.0, 0.0, 0.0};

    /// Вектор продольной оси подвижной единицы
    dvec3 v_orth = {0.0, 1.0, 0.0};

    /// Вектор вертикальной оси подвижной единицы
    dvec3 v_up = {0.0, 0.0, 1.0};

    /// Точка изгиба в начале рукава, в локальных координатах подвижной единицы, м
    dvec3 root_pos = {0.0, 0.0, 0.0};

    /// Существующее конструктивное отклонение рукава от продольной оси
    /// (<0 - повёрнут влево, >0 - повёрнут вправо), радиан
    double existing_side_angle = 0.0;
    /// Отклонение рукава от продольной оси, в свободном состоянии, радиан
    double free_side_angle = 0.0;
    /// Отклонение рукава от продольной оси, в закреплённом на подвесе состоянии, радиан
    double fixed_side_angle = 0.0;
    /// Несовпадение поверхности смыкания стандартной соединительной головки с осью рукава,
    /// требующее развести их на 1 см влево, м
    double side_gap = -0.01;

    /// Существующее конструктивное свешивание рукава вниз от продольной оси, радиан
    double existing_down_angle = 45.0 / 180.0 * Physics::PI;
    /// Свешивание рукава вниз от продольной оси, в свободном состоянии, радиан
    double free_down_angle = 60.0 / 180.0 * Physics::PI;
    /// Свешивание рукава вниз от продольной оси, в закреплённом на подвесе состоянии, радиан
    double fixed_down_angle = 60.0 / 180.0 * Physics::PI;

    /// Существующий конструктивный изгиб вверх соединительной головки рукава, радиан
    double existing_head_up_angle = 24.0 / 180.0 * Physics::PI;
    /// Изгиб рукава вверх перед соединительной головкой, в свободном состоянии, радиан
    double free_head_up_angle = 12.0 / 180.0 * Physics::PI;
    /// Изгиб рукава вверх перед соединительной головкой, в закреплённом на подвесе состоянии, радиан
    double fixed_head_up_angle = 24.0 / 180.0 * Physics::PI;
    /// Изгиб рукава вверх перед соединительной головкой, в состоянии готовности к соединению, радиан
    double ready_to_connect_head_up_angle = 36.0 / 180.0 * Physics::PI;

    /// Коэффициент к скорости поворотов рукава
    double angle_omega = 8.0;
    /// Коэффициент к скорости колебаний свешивания рукава в свободном состоянии
    double free_angle_omega = 8.0;
    /// Коэффициент затухания колебаний свешивания рукава в свободном состоянии
    double free_angle_beta = 1.0;

    bool check_angles_ready(double ref_side_angle,
                            double ref_down_angle,
                            double ref_head_up_angle);

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t) override;

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg) override;
};

#endif // PNEUMO_HOSE_H
