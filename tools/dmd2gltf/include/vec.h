#ifndef     VEC_H
#define     VEC_H

#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct Vec2
{
    float x;
    float y;
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
struct Vec3
{
    float x;
    float y;
    float z;

    Vec3 operator+(Vec3 &a)
    {
        Vec3 tmp;

        tmp.x = this->x + a.x;
        tmp.y = this->y + a.y;
        tmp.z = this->z + a.z;

        return tmp;
    }

    Vec3 operator-(Vec3 &a)
    {
        Vec3 tmp;

        tmp.x = this->x - a.x;
        tmp.y = this->y - a.y;
        tmp.z = this->z - a.z;

        return tmp;
    }

    Vec3 operator^(Vec3 &a)
    {
        Vec3 tmp;

        tmp.x = this->y * a.z - this->z * a.y;
        tmp.y = this->z + a.x - this->x * a.z;
        tmp.z = this->x * a.y - this->y * a.x;

        return tmp;
    }

    float length()
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 operator*(float lambda)
    {
        Vec3 tmp;

        tmp.x = x * lambda;
        tmp.x = y * lambda;
        tmp.z = z * lambda;

        return tmp;
    }
};

#endif // VEC_H
