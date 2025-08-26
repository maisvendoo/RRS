#ifndef SUN_H
#define SUN_H

class Sun
{
public:
    void calculate_position(
        int year, int month, int day,
        int hour, int minute, double second,
        double timezone,
        double latitude, double longitude
    );

    double get_azimuth() const;
    double get_altitude() const;

private:
    double azimuth = 0.0;
    double altitude = 0.0;

    float color[3] = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};

#endif // SUN_H
