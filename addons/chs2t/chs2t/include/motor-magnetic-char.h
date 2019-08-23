//------------------------------------------------------------------------------
//
//      Магистральный пассажирский электровоз постоянного тока ЧС2т.
//      Дополнение для Russian Railway Simulator (RRS)
//
//      (c) RRS development team:
//          Дмитрий Притыкин (maisvendoo),
//          Николай Авилкин (avilkin.nick)
//
//      Дата: 21/08/2019
//
//------------------------------------------------------------------------------
#ifndef     MOTOR_MAGNETIC_CHAR_H
#define     MOTOR_MAGNETIC_CHAR_H

#include    <QString>
#include    <vector>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class MotorMagneticChar
{
public:

    MotorMagneticChar();

    ~MotorMagneticChar();

    void load(const std::string &path);

    double getValue(double I);

private:

    struct point_t
    {
        double  current;
        double  value;
    };

    std::vector<point_t> points;

    point_t findPoint(double I, point_t &next_point);

    double  interpolate(double I);
};

#endif // MOTOR_MAGNETIC_CHAR_H
