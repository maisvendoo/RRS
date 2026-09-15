#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H

#include <QLabel>
#include    "image-widget.h"



class Speedometer : public QLabel
{

public:
    Speedometer(QSize size, QString cfg_path, QWidget *parent = Q_NULLPTR);

    void setSpeeds(int speed, int speedLimit, int speedNextLimit);

private:
    std::vector<QPointF> speed_coordsOutScale;
    std::vector<QPointF> speed_coordsInsideScale;

    int num_speed_ = 0;
    int num_speedLimit_ = 0;
    int num_speedNextLimit_ = 0;

    int old_num_speed_ = -1;
    int old_num_speedLimit_ = -1;
    int old_num_speedNextLimit_ = -1;


    void drawArc_(int num_speed, int num_speedLimit, int num_speedNextLimit);

    void loadScalePontsCoolrds_(QString txt_path, std::vector<QPointF> &vec);


};

#endif // SPEEDOMETER_H
