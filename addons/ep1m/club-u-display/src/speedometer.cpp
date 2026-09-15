#include "speedometer.h"

#include <QPainter>
#include <QVector>
#include <QFile>

const int     indSize = 7;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

Speedometer::Speedometer(QSize size, QString cfg_path, QWidget* parent)
    : QLabel(parent)
{
    this->resize(size);
    //this->setStyleSheet("border: 1px solid red;");

    loadScalePontsCoolrds_(cfg_path + "speed-coordinatesOutScale.txt", speed_coordsOutScale);
    loadScalePontsCoolrds_(cfg_path + "speed-coordinatesInsideScale.txt", speed_coordsInsideScale);

    QPixmap pix = QPixmap(this->size());
    pix.fill(Qt::transparent);
    QPainter paint(&pix);
    paint.setRenderHint(QPainter::Antialiasing, true);


    // ограничение скорости
    paint.setPen(QPen( QColor(Qt::red), indSize, Qt::SolidLine, Qt::RoundCap ));
    for (int i = 0; i < speed_coordsOutScale.size(); ++i)
    {
        paint.drawPoint(speed_coordsOutScale[i]);
    }

    // скорость
    paint.setPen(QPen( QColor(Qt::green), indSize, Qt::SolidLine, Qt::RoundCap ));
    for (int i = 0; i < speed_coordsInsideScale.size(); ++i)
    {
        paint.drawPoint(speed_coordsInsideScale[i]);
    }


    paint.end();
    this->setPixmap(pix);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Speedometer::setSpeeds(int speed, int speedLimit, int speedNextLimit)
{
    num_speed_ = std::min(speed / 5, static_cast<int>(speed_coordsInsideScale.size()) - 1);
    num_speedLimit_ = std::min(speedLimit / 5, static_cast<int>(speed_coordsOutScale.size()) - 1);
    num_speedNextLimit_ = std::min(speedNextLimit / 5, static_cast<int>(speed_coordsOutScale.size()) - 1);

    if ((num_speed_ == old_num_speed_) &&
        (num_speedLimit_ == old_num_speedLimit_) &&
        (num_speedNextLimit_ == old_num_speedNextLimit_))
    {
        return;
    }

    drawArc_(num_speed_, num_speedLimit_, num_speedNextLimit_);

    old_num_speed_ = num_speed_;
    old_num_speedLimit_ = num_speedLimit_;
    old_num_speedNextLimit_ = num_speedNextLimit_;
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Speedometer::drawArc_(int num_speed, int num_speedLimit, int num_speedNextLimit)
{
    QPixmap pix = QPixmap(this->size());
    pix.fill(Qt::transparent);
    QPainter paint(&pix);
    paint.setRenderHint(QPainter::Antialiasing, true);


    if (num_speedNextLimit >= 0)
    {
        // следующее ограничение скорости
        paint.setPen(QPen( QColor(Qt::yellow), indSize, Qt::SolidLine, Qt::RoundCap ));
        paint.drawPoint(speed_coordsOutScale[num_speedNextLimit]);
    }
    if (num_speedLimit >= 0)
    {
        // ограничение скорости
        paint.setPen(QPen( QColor(Qt::red), indSize, Qt::SolidLine, Qt::RoundCap ));
        paint.drawPoint(speed_coordsOutScale[num_speedLimit]);
    }

    // скорость
    paint.setPen(QPen( QColor(Qt::green), indSize, Qt::SolidLine, Qt::RoundCap ));
    for (int i = 0, n = num_speed + 1; i < n; ++i)
    {
        paint.drawPoint(speed_coordsInsideScale[i]);
    }


    paint.end();
    this->setPixmap(pix);
}



//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Speedometer::loadScalePontsCoolrds_(QString txt_path, std::vector<QPointF> &vec)
{
    QFile fileTxt(txt_path);

    if (!QFile::exists(fileTxt.fileName()))
        return;

    if (fileTxt.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        vec.clear();
        while (!fileTxt.atEnd())
        {
            QString str = fileTxt.readLine();
            QStringList strList = str.split(" ");
            double x = strList[0].toDouble();
            double y = strList[1].toDouble();
            vec.push_back(QPointF(x, y));
        }
        fileTxt.close();
    }

}

