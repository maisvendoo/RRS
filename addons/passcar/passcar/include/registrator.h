#ifndef     REGISTRATOR_H
#define     REGISTRATOR_H

#include    <QString>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Registrator
{
public:

    Registrator(QString log_name = "ra3.log", double timeout = 0.1);

    ~Registrator();

    void print_msg(QString msg, double t, double dt);

private:

    QString log_name;

    double timeout;

    bool first_msg;

    double tau;

    QString logs_dir;

};

#endif // REGISTRATOR_H
