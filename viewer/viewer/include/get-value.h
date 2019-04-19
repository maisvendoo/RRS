#ifndef     GET_VALUE_H
#define     GET_VALUE_H

#include    <sstream>

/*!
 * \fn
 * \brief Get value from string
 */
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <class T>
bool getValue(const std::string &str, T &value)
{
    std::istringstream ss(str);

    try
    {
        ss >> value;
    }
    catch (std::exception &)
    {
        return false;
    }

    return true;
}

#endif // GETVALUE_H
