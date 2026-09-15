#include "text-paint.h"


#include <QPainter>

#include <QFontDatabase>

const std::map<TextPaint::DisplayType, QString> displayFonts = {
    {TextPaint::LED_6X8_DOTS, ":/rcc/led-6x8-dots"},
    {TextPaint::LED_7SEGMENT, ":/rcc/led-7seg-italic"}
};



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
TextPaint::TextPaint(QSize _size, QWidget *parent) : QLabel(parent)
{
    this->resize(_size);
    //this->setStyleSheet("border: 1px solid red;");

    img_ = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);

}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TextPaint::setFonts(int fontSize, Qt::GlobalColor color, DisplayType type, int txtWeight)
{
    fontSize_ = fontSize;
    color_ = color;
    txtWeight_ = txtWeight;

    int id = QFontDatabase::addApplicationFont(displayFonts.at(type)); //путь к шрифту
    font_ = QFont(QFontDatabase::applicationFontFamilies(id).at(0), fontSize_, txtWeight_);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TextPaint::setParams(int countCell, int deltaX, bool symbolIsNull, bool rightleftText)
{
    countCell_ = countCell;
    deltaX_ = deltaX;
    symbolIsZero_ = symbolIsNull;
    rightleftText_ = rightleftText;
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TextPaint::setPointForDigit(int x, int y)
{
    flagSetPoint_ = true;
    pointX_ = x;
    pointY_ = y;
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TextPaint::setText(QString txt)
{
    drawText_(txt);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void TextPaint::drawText_(QString txt)
{
    img_.fill(Qt::transparent);
    QPixmap pix = QPixmap::fromImage(img_);
    QPainter paint(&pix);
    //paint.setRenderHint(QPainter::Antialiasing, true);
    paint.setFont(font_);
    paint.setPen(color_);

    int txt_size = static_cast<int>(txt.size());
    int symbols = std::min(countCell_, txt_size);
    if (rightleftText_)
    {
        int i = 1;

        while (i <= symbols)
        {
            int posX = this->width() - i * deltaX_;
            int k = txt_size - i;

            paint.drawText(posX, this->height(), QString(txt[k]));
            ++i;
        }

        if (symbolIsZero_)
        {
            while (i <= countCell_)
            {
                int posX = this->width() - i * deltaX_;

                paint.drawText(posX, this->height(), "0");
                ++i;
            }
        }
    }
    else
    {
        for (int i = 0; i < symbols; ++i)
        {
            int posX = i * deltaX_;

            paint.drawText(posX, this->height(), QString(txt[i]));
        }
    }

    //
    if (flagSetPoint_)
    {
        paint.setPen(QPen(QColor(color_), 4));
        paint.setRenderHint(QPainter::Antialiasing, true);
        paint.drawPoint(pointX_, pointY_);
    }


    paint.end();
    this->setPixmap(pix);
}
