#ifndef     SIGNALING_H
#define     SIGNALING_H

#include    <QObject>

#include    "signaling-export.h"

#include    "block-section.h"

#include    "CfgReader.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SIGNALING_EXPORT Signaling : public QObject
{
    Q_OBJECT

public:

    Signaling(QObject *parent = Q_NULLPTR);

    ~Signaling();

    virtual void step(double t, double dt);

    bool load_signals(int dir, const QString &route_dir);

protected:

    /// Блок-участки на маршруте
    std::vector<BlockSection *> sections;

    /// Парсинг конфига сигналов
    void signals_parse(CfgReader &cfg);

    /// Создание сигнала соотвествующего типа
    Signal *createSignal(const QString &type, const QString &liter);
};

#endif // SIGNALING_H
