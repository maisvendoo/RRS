#ifndef     VECTOR3_H
#define     VECTOR3_H

#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Vec3
{
public:

    float x;
    float y;
    float z;

    Vec3()
        : x(0.0f)
        , y(0.0f)
        , z(0.0f)
    {

    }

    float length()
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 operator+(Vec3 &b)
    {
        Vec3 tmp;

        tmp. x = this->x + b.x;
        tmp. y = this->y + b.y;
        tmp. z = this->z + b.z;

        return tmp;
    }

    Vec3 operator-(Vec3 &b)
    {
        Vec3 tmp;

        tmp. x = this->x - b.x;
        tmp. y = this->y - b.y;
        tmp. z = this->z - b.z;

        return tmp;
    }

    Vec3 orth()
    {
        Vec3 tmp;

        float len = this->length();

        if (len == 0.0f)
            return tmp;
        else
        {
            tmp.x = this->x / len;
            tmp.y = this->y / len;
            tmp.z = this->z / len;

            return tmp;
        }
    }

private:


};

#endif // VECTOR3_H
