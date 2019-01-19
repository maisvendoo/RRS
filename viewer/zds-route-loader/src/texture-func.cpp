#include    "texture-func.h"

struct pixel_t
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

void convertTexture(osg::Image *image)
{
    unsigned char *data = image->data();

    for (unsigned int i = 0; i < image->getTotalDataSize(); i += 4)
    {
        pixel_t pixel;

        pixel.r = data[i + 2];
        pixel.g = data[i + 1];
        pixel.b = data[i];
        pixel.a = data[i + 3];

        pixel.r = pixel.r * pixel.a / 255;
        pixel.g = pixel.g * pixel.a / 255;
        pixel.b = pixel.b * pixel.a / 255;

        data[i + 2] = pixel.r;
        data[i + 1] = pixel.g;
        data[i] = pixel.b;
    }
}
