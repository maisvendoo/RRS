#ifndef     RANGE_HANDLER_H
#define     RANGE_HANDLER_H

#include    <control-handler.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class RangeHandler : public ControlHandler
{
public:

    explicit RangeHandler(QObject *parent = nullptr);

    /// Обработка клавиатурного ввода
    virtual void processKeyInput(const std::set<uint16_t>& pressed_keys) override;

    /// Обработка мышиного ввода
    virtual void processMouseInput(uint32_t button, bool is_pressed) override;

    virtual bool load_config(CfgReader &cfg, QDomNode secNode) override;

    QString getUsage() const override;

protected:

    struct position_t
    {
        float value = 0.0f;
        uint16_t hotKey = 0;
    };

    /// Модификатор для установки в заданную позицию
    QString positionModkey = "";

    /// Клавиши и позиции для переключения
    std::vector<position_t> posKeys;

    uint16_t keyCodeInc = 0;          // клавиша увеличения позиции
    QString  keyModIncName = "";      // модификатор увеличения (опционально)
    uint16_t keyCodeDec = 0;          // клавиша уменьшения позиции
    QString  keyModDecName = "";      // модификатор уменьшения (опционально)

    /// Сбросная клавиша
    uint16_t resetKey = 0;
    /// Сбросной модификатор
    QString resetModkey = "";

    /// Позиция, до которой будет выполняться сброс
    float posForReset = 0.0f;

    /// Клавиша мыши увеличения позиции
    QString incButtonName = "LEFT_BUTTON";
    /// Клавиша мыши уменьшения позиции
    QString decButtonName = "RIGHT_BUTTON";
};

#endif
