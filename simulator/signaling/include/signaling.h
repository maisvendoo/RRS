#ifndef     SIGNALING_H
#define     SIGNALING_H

#include    <QObject>

#include    "signaling-export.h"

#include    "block-section.h"

#include    "CfgReader.h"

#include    "alsn-struct.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class SIGNALING_EXPORT Signaling : public QObject
{
    Q_OBJECT

public:

    Signaling(QObject *parent = Q_NULLPTR);

    ~Signaling();

    /// Шаг моделирования устройств СЦБ
    virtual void step(double t, double dt);

    /// Инициализация СЦБ
    bool init(int dir, const QString &route_dir);    

    /// Получить код АЛСН по координате ПС
    alsn_info_t getALSN(double coord);

public slots:

    /// Проверка занятости блок-участков, в зависимости от координат ПС
    void set_busy_sections(double x);

protected:

    /// Направление движения на участке
    int dir;

    /// Блок-участки на маршруте
    std::vector<BlockSection *> sections;

    /// Парсинг конфига сигналов
    void signals_parse(CfgReader &cfg);

    /// Создание сигнала соотвествующего типа
    Signal *createSignal(const QString &type, const QString &liter);

    /// Инициализация связей между сигналами
    void init_signal_links();

    /// Поиск блок-участка по координатам
    BlockSection *findSection(double x) const;
};

#endif // SIGNALING_H
