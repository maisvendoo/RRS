#ifndef     TRAJECTORY_H
#define     TRAJECTORY_H

#include    <QObject>

#include    <track.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Trajectory : public QObject
{
public:

    Trajectory(QObject *parent = Q_NULLPTR);

    ~Trajectory();

private:

    std::vector<track_t>    tracks;
};

#endif
