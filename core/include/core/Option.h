#ifndef CORE_OPTION_H
#define CORE_OPTION_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template <class T>
struct option_t
{
public:
    T value;

    option_t()
        : value(T())
        , is_present(false)
    {
    }

    T get()
    {
        return is_present ? value : T();
    }

    void operator=(const T& value)
    {
        if (value == T())
        {
            is_present = false;
            return;
        }

        this->value = value;
        is_present = true;
    }

    bool isPresent() const
    {
        return is_present;
    }

private:
    bool is_present;
};

#endif // CORE_OPTION_H
