#include    "dmd-container.h"

DMDContainer::DMDContainer()
    : current_line(0)
    , content({})
{

}

DMDContainer::~DMDContainer()
{

}

bool DMDContainer::load(std::ifstream &fin)
{
    bool result = true;

    if (fin.is_open())
    {
        while (!fin.eof())
        {
            std::string line;
            std::getline(fin, line);
            content.push_back(line);
        }
    }
    else
        result = false;

    return result;
}

bool DMDContainer::isEnd() const
{
    return current_line == content.size();
}

std::string DMDContainer::getLine()
{
    return content[current_line++];
}
