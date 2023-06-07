#ifndef     HOSE369A_H
#define     HOSE369A_H

#include    "pneumo-hose-epb.h"

enum
{
    EPB_LINES_NUM = 2,      ///< Количество линий ЭПТ
    EPB_WORK_LINE = 0,      ///< Линия управления ЭПТ
    EPB_CONTROL_LINE = 1,   ///< Линия контроля ЭПТ
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class Hose369A : public PneumoHoseEPB
{
public:

    Hose369A(QObject *parent = Q_NULLPTR);

    ~Hose369A();

    virtual void step(double t, double dt);

private:

    /// Признак использования рукава на локомотиве
    /// (на локомотивах у рукавов, закреплённых на подвесе, линии ЭПТ размыкаются)
    bool is_hose_at_loco;

    void load_config(CfgReader &cfg);
};

#endif // HOSE369A_H
