#ifndef     PNEUMO_HOSE_H
#define     PNEUMO_HOSE_H

#include    "device.h"

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

    /// Задать управляющую клавишу для команды соединить рукава
    void setKeySymbolConnect(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор для команды соединить рукава
    void setKeyModifierConnect(std::uint16_t key_modifier);

    /// Задать управляющую клавишу для команды разъединить рукава
    void setKeySymbolDisconnect(std::uint16_t key_symbol);

    /// Задать клавишу-модификатор для команды разъединить рукава
    void setKeyModifierDisconnect(std::uint16_t key_modifier);

    void setControl(std::set<uint16_t>* keys = nullptr,
                    control_signals_t* control_signals = nullptr) override;

    /// Соединить рукава
    void connect();

    /// Разъединить рукава
    void disconnect();

    /// Состояние соединения рукавов
    bool isConnected() const;

    /// Задать давление в рукаве
    void setPressure(double value);

    /// Задать коэффициент перетока через рукав
    void setFlowCoeff(double value);

    /// Задать длину рукава, м
    void setLength(double value);

    /// Задать смещение точки крепления рукава в сторону, м
    void setShiftSide(double value);

    /// Задать координату точки крепления руква на треке пути, м
    void setCoord(double value);

    /// Получить поток через рукав
    double getFlow() const;

    /// Получить угол отклонения рукава в сторону соседнего, радиан
    double getSideAngle() const;

    /// Получить угол свешивания рукава вниз с учётом натяжения от соседнего, радиан
    double getDownAngle() const;

    virtual void step(double t, double dt) override;

    enum {
        NUM_SOUNDS = 3,
        PIPE_DRAIN_FLOW_SOUND = 0, ///< Звук опорожнения магистрали через рукав
        CONNECT_SOUND = 1,   ///< Звук соединения рукавов
        DISCONNECT_SOUND = 2 ///< Звук рассоединения рукавов
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = PIPE_DRAIN_FLOW_SOUND) const override;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = PIPE_DRAIN_FLOW_SOUND) const override;

protected:

    /// Коэффициент громкости озвучки к потоку опорожнения магистрали через рукав
    double K_sound = 1.5;

    /// Управление рукавом с клавиатуры: 0 - рассоединить, 1 - оставить текущее состояние, 2 - соединить
    SwitcherControl ref_state = SwitcherControl(3);

    /// Звук опорожнения магистрали через рукав
    sound_state_t atm_flow_sound = sound_state_t();

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t) override;

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg) override;
};

#endif // PNEUMO_HOSE_H
