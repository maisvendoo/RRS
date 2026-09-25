#ifndef     IO_CONTROLLER_H
#define     IO_CONTROLLER_H

#include    <io-controller-export.h>
#include    <control-handler.h>
#include    <QObject>
#include    <set>

#include    <dual-key-hash.h>
#include    <QMap>

class CfgReader;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class IO_CONTROLLER_EXPORT IOController : public QObject
{
    Q_OBJECT

public:

    IOController(QObject *parent = nullptr);

    ~IOController() = default;

    /// Задать нажатую клавишу
    void setPressedKey(uint16_t keyBase);

    /// Задать отпущенную клавишу
    void setReleasedKey(uint16_t keyBase);

    /// Шаг контролов
    virtual void step(float t, float dt, const std::vector<float> *server_signals);

    /// Загрузить конфиг
    virtual bool load_config(CfgReader &cfg);

    /// Создать карту анимаций
    void create_animations_map(const QStringList &anim_dirs);

    /// Инициализация
    virtual void init() {}

    /// Установить индекс управляемой ПЕ
    void setVehicleIndex(int vehicle_idx);

    /// Установить индекс активной кабины
    void setActirveCabineIndex(int cab_idx)
    {
        cabine_idx = cab_idx;
    }

    /// Поиск обработчика по имени объекта
    bool findControlHandler(const std::string &node_name, ControlHandler *&handler) const;

signals:

    void sigSendVehicleControlCommand(const QByteArray &data);

protected:

    /// Массив нажатых клавиш
    std::set<uint16_t> _pressed_keys;    

    /// Здесь обеспечивается доступ к обработчику контрола
    /// как по коду нажатой кавиши, так и по имени объекта, кликнутого мышью
    std::vector<DualKeyHash<uint16_t, QString, ControlHandler *>> control_handlers;

    /// Число кабин
    int cabs_num = 0;
    /// Активная кабина
    int cabine_idx = 0;
    /// Индекс управляемой ПЕ
    int vehicle_idx = 0;

    /// Обработка управления с клавиатуры в кастомных модулях
    virtual void processKeyboardInput(std::set<uint16_t> &pressed_keys);

    /// Создать обработчик контрола
    virtual ControlHandler *create_handler(QString type, QDomNode secNode, CfgReader &cfg);

private:

    /// Маппинг: имя 3D-объекта → ID сигнала обратной связи из analogSignal
    QMap<QString, uint16_t> animation_signals_map;

    /// Обработка клавиатурного управления (Общая для всех часть)
    void processKeyBoardInput();

    /// Обработка клавиатурного ввода
    void keyboardInputProcess(std::set<uint16_t> &pressed_keys);

    /// Сформировать строку с посказкой горячей клавиши
    void getHotkeysString(const QString &keyName, ControlHandler *ctrl_handler);    
};

#endif
