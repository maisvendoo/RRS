#include    "signaling.h"

#include    <QDir>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signaling::Signaling(QObject *parent) : QObject(parent)
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

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool Signaling::load_signals(int dir, const QString &route_dir)
{
    QString file_name = "signals";

    if (dir > 0)
        file_name += "1.conf";
    else
        file_name += "2.conf";

    QString path = QDir::toNativeSeparators(route_dir) +
            QDir::separator() +
            file_name;

    CfgReader cfg;

    if (!cfg.load(path))
        return false;

    signals_parse(cfg);

    return !sections.empty();
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

        section = section->getNextSection();

        secNode = cfg.getNextSection();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Signal *Signaling::createSignal(const QString &type, const QString &liter)
{
    return Q_NULLPTR;
}
