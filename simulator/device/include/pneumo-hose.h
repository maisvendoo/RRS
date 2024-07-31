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
    PneumoHose(int key_code = 0, QObject *parent = Q_NULLPTR);

    /// Деструктор
    ~PneumoHose();

    /// Задать управляющую клавишу
    void setKeyCode(int key_code);

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

    virtual void step(double t, double dt);

    enum {
        NUM_SOUNDS = 3,
        PIPE_DRAIN_FLOW_SOUND = 0, ///< Звук опорожнения магистрали через рукав
        CONNECT_SOUND = 1,   ///< Звук соединения рукавов
        DISCONNECT_SOUND = 2 ///< Звук рассоединения рукавов
    };
    /// Состояние звука
    virtual sound_state_t getSoundState(size_t idx = PIPE_DRAIN_FLOW_SOUND) const;

    /// Сигнал состояния звука
    virtual float getSoundSignal(size_t idx = PIPE_DRAIN_FLOW_SOUND) const;

protected:

    /// Код управляющей клавиши
    int keyCode;

    /// Флаг вызова команд управления соединением рукавов
    bool is_ref_state_command;

    /// Коэффициент громкости озвучки к потоку опорожнения магистрали через рукав
    double K_sound = 1.5;

    /// Звук опорожнения магистрали через рукав
    sound_state_t atm_flow_sound = sound_state_t();

    virtual void ode_system(const state_vector_t &Y,
                            state_vector_t &dYdt,
                            double t);

    virtual void stepKeysControl(double t, double dt);

    /// Загрузка параметров из конфигурационного файла
    virtual void load_config(CfgReader &cfg);
};

/*!
 * \typedef
 * \brief getPneumoHose() signature
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
typedef PneumoHose* (*GetPneumoHose)();

/*!
 * \def
 * \brief Macro for getPneumoHose() generation
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
#define GET_PNEUMO_HOSE(ClassName) \
    extern "C" Q_DECL_EXPORT PneumoHose *getPneumoHose() \
    {\
        return new (ClassName)(); \
    }

/*!
 * \fn
 * \brief Load PneumoHose from library
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
extern "C" Q_DECL_EXPORT PneumoHose *loadPneumoHose(QString lib_path);

#endif // PNEUMO_HOSE_H
