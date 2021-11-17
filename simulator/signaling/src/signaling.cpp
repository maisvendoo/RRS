#include    "signaling.h"

#include    <QDir>

#include    "line-signal.h"

#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signaling::Signaling(QObject *parent) : QObject(parent)
  , dir(1)
  , is_ready(false)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signaling::~Signaling()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signaling::step(double t, double dt)
{
    for (BlockSection *section : sections)
    {
        section->step(t, dt);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Signaling::init(int dir, const QString &route_dir)
{
    QString file_name = "signals";

    if (dir > 0)
        file_name += "1.xml";
    else
        file_name += "2.xml";

    this->dir = dir;

    QString path = QDir::toNativeSeparators(route_dir) +
            QDir::separator() +
            file_name;

    CfgReader cfg;

    if (!cfg.load(path))
        return false;

    signals_parse(cfg);

    init_signal_links();

    return is_ready = !sections.empty();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signaling::check_busy_sections(double x)
{
    size_t left_idx = 0;
    size_t right_idx = sections.size() - 1;
    size_t idx = (left_idx + right_idx) / 2;

    while (left_idx != right_idx)
    {
        if (dir > 0)
        {
            if (x < sections[idx]->getBeginCoord())
                right_idx = idx;
            else
                left_idx = idx;
        }

        if (dir < 0)
        {
            if (x > sections[idx]->getBeginCoord())
                right_idx = idx;
            else
                left_idx = idx;
        }

        idx = (left_idx + right_idx) / 2;
    }

    sections[idx]->setBusy(true);
    sections[idx]->getPrevSection()->setBusy(false);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signaling::signals_parse(CfgReader &cfg)
{
    QDomNode secNode = cfg.getFirstSection("Signal");

    BlockSection *section = new BlockSection;

    while (!secNode.isNull())
    {
        double ordinate = 0;
        cfg.getDouble(secNode, "Ordinate", ordinate);
        section->setEndCoord(ordinate);
        section->setNextSection(new BlockSection);
        section->getNextSection()->setBeginCoord(ordinate);

        QString signal_type = "";
        cfg.getString(secNode, "Type", signal_type);

        QString signal_liter = "";
        cfg.getString(secNode, "Liter", signal_liter);

        section->getNextSection()->setSignal(createSignal(signal_type, signal_liter));

        sections.push_back(section);

        QString msg = QString("Loaded block section: L = %1")
                .arg(section->getLenght());

        Journal::instance()->info(msg);

        section = section->getNextSection();

        secNode = cfg.getNextSection();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signal *Signaling::createSignal(const QString &type, const QString &liter)
{
    Signal *signal = Q_NULLPTR;

    // ДЛЯ ТЕСТА!!! Все сигналы проходные
    signal = new LineSignal;

    signal->setType(type);
    signal->setLiter(liter);

    return signal;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Signaling::init_signal_links()
{
    if (sections.empty())
        return;

    for (size_t i = 0; i < sections.size() - 1; ++i)
    {
        connect(sections[i+1]->getSignal(), &Signal::sendClosedState,
                sections[i]->getSignal(), &Signal::slotRecvPreviosState);
    }
}
