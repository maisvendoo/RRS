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

    Vec3(float x, float y, float z)
        : x(x)
        , y(y)
        , z(z)
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

    Vec3 operator^(Vec3 &b)
    {
        Vec3 tmp;

        tmp.x = this->y * b.z - b.y * this->z;
        tmp.y = b.x * this->z - this->x * b.z;
        tmp.z = this->x * b.y - b.y * this->x;

        return tmp;
    }

    static float dot_product(Vec3 &a, Vec3 &b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

private:


};

#endif // VECTOR3_H
