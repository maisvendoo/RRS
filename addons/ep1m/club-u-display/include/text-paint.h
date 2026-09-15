#ifndef TEXTPAINT_H
#define TEXTPAINT_H


#include <QLabel>


class TextPaint : public QLabel
{

public:
    TextPaint(QSize _size, QWidget *parent = Q_NULLPTR);

    enum DisplayType
    {
        LED_6X8_DOTS,
        LED_7SEGMENT
    };

    void setFonts(int fontSize, Qt::GlobalColor color, DisplayType type = LED_6X8_DOTS, int txtWeight = 50);
    // Параметры символьных индикаторов: количество ячеек в строке, смещение по горизонтали,
    // заполнение пробелами (false) или нулями (true, по умолчанию),
    // заполнять текст от левого края (false) или от правого (true, по умолчанию)
    void setParams(int countCell, int deltaX, bool symbolIsZero = true, bool rightleftText = true);
    // Установить точку-разделитель числа
    void setPointForDigit(int x, int y);

    void setText(QString txt);



private:
    QImage img_;

    QFont font_;

    int fontSize_ = 20;
    Qt::GlobalColor color_ = Qt::green;
    int txtWeight_ = 50;    // жирность текста
    int countCell_ = 1;     // количество ячеек под текст
    int deltaX_ = 12;       // дельта X для символов в ячейках
    bool symbolIsZero_ = true;  // null или 0 на дисплее
    bool rightleftText_ = true; // текст справа налево

    // точка-разделитель числа
    bool flagSetPoint_ = false;
    int pointX_ = -1;
    int pointY_ = -1;


    void drawText_(QString txt);
};

#endif // TEXTPAINT_H
