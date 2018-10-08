
#ifndef     DMD_CONTAINER_H
#define     DMD_CONTAINER_H

#include    <vector>
#include    <string>
#include    <iostream>
#include    <fstream>

class DMDContainer
{
public:

    DMDContainer();
    virtual ~DMDContainer();

    bool load(std::ifstream &fin);

    bool isEnd() const;

    std::string getLine();

private:

    size_t                      current_line;
    std::vector<std::string>    content;
};

#endif // DMDCONTAINER_H
