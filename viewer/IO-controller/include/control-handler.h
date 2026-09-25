#ifndef     CONTROL_HANDLER_H
#define     CONTROL_HANDLER_H

#include "qdom.h"
#include    <QObject>

#include    <set>

#include    <dual-key-hash.h>
#include    <io-controller-input.h>
#include    <io-controller-keymap.h>

#include    <CfgReader.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class ControlHandler : public QObject
{
    Q_OBJECT

public:

    /// Индекс управляемой ПЕ
    int controlled_vehicle_idx = -1;
    /// Индекс активной кабины управляемой ПЕ
    int cabine_idx = -1;
    /// Идентификатор сигнала управления в массиве сигналов ПЕ
    uint16_t id = 0;
    /// Значение управляющего сигнала
    float value = 0.0f;

    /// Код назначенной клавиши
    uint16_t keyCode = 0;
    /// Имя модификатора включения
    QString keyModOnName = "";
    /// Имя модификатора выключения
    QString keyModOffName = "";
    /// Имя объекта в 3D-модели
    QString contolledObjectName = "";
    /// Имя контрола
    QString name = "";
    /// Тип контрола
    QString type = "";
    /// Описание контрола
    QString description = "";
    /// Использование контрола
    QString usage = "";
    /// Описание горячей клавиши
    QString hot_keys = "";

    explicit ControlHandler(QObject *parent = nullptr) : QObject(parent)
    {

    }

    virtual ~ControlHandler() = default;

    void setFeedbackSignals(const std::vector<float> *feedback_signals)
    {
        this->feedback_signals = feedback_signals;
    }

    void setAnimationSignalsMap(const QMap<QString, uint16_t> *animation_signals_map)
    {
        this->animation_signals_map = animation_signals_map;
    }

    void setControlInputs(std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>>* inputs)
    {
        ctrl_inputs = inputs;
    }

    /// Загрузка конфигурации из IOControllerConfig
    virtual bool load_config(CfgReader &cfg, QDomNode secNode);

    /// Обработка клавиатурного ввода
    virtual void processKeyInput(const std::set<uint16_t>& pressed_keys) = 0;

    /// Обработка мышиного ввода
    virtual void processMouseInput(uint32_t button, bool is_pressed) = 0;

    virtual void step(float dt) {};

signals:

    void sigSendControlCommand(const QByteArray &data);

protected:

    /// Указатель на DualKeyHash IOController (данные и состояние)
    std::vector<DualKeyHash<uint16_t, QString, io_control_input_t>>* ctrl_inputs = nullptr;

    const std::vector<float> *feedback_signals = nullptr;

    const QMap<QString, uint16_t> *animation_signals_map = nullptr;

    /// Получить текущее значение сигнала по имени 3D-объекта
    float getSignalValueByName(const QString& objectName) const;

    /// Получить текущее значение сигнала по ID контрола
    float getSignalValueByID(uint16_t control_id, int cab_idx) const;

    //bool getKeyState(const std::set<uint16_t>& keys, uint16_t key);

    bool isKeyModifier(const std::set<uint16_t>& keys, const QString& modName);

    void sendControlSignal();

    QByteArray serialize() const
    {
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);

        // Серверу передаем только эти значения
        stream << controlled_vehicle_idx;
        stream << cabine_idx;
        stream << id;
        stream << value;

        return data;
    }

    void deserialize(QByteArray &data)
    {
        QDataStream stream(&data, QIODevice::ReadOnly);

        stream >> controlled_vehicle_idx;
        stream >> cabine_idx;
        stream >> id;
        stream >> value;
    }

    bool toBool() const
    {
        return static_cast<bool>(value);
    }
};

#endif
