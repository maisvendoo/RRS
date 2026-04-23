#include    <ktx.h>
#include    <nlohmann/json.hpp>
#include    <stb_image.h>
#include    <cmdparser.hpp>

int main()
{
    int w = 0, h = 0, ch = 0;
    stbi_uc *data = stbi_load("image.jpg", &w, &h, &ch, 0);

    return 0;
}
