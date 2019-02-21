#ifndef     TRIGGER_H
#define     TRIGGER_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template<class T>
class Trigger
{
public:

    Trigger() : lock(false) {}
    virtual ~Trigger() {}

protected:

    bool lock;

    virtual void operate(T &)
    {

    }    

public:

    void process(bool key, T &value)
    {
        if (key)
        {
            if (!lock)
            {
                operate(value);
                lock = true;
            }
        }
        else
        {
            lock = false;
        }
    }
};

#endif // TRIGGER_H
