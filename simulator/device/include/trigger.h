#ifndef     TRIGGER_H
#define     TRIGGER_H

class   Trigger
{
public:

    Trigger();

    ~Trigger();

    bool getState() const;

    void set();

    void reset();

private:

    bool state;
};

#endif // TRIGGER_H
