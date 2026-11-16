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

    Hose369A(QObject *parent = nullptr);

    ~Hose369A();

    void preStep(state_vector_t& Y, double t) override;

private:

    /// Признак использования рукава на локомотиве
    /// (на локомотивах у рукавов, закреплённых на подвесе, линии ЭПТ размыкаются)
    bool is_hose_at_loco;

    void load_config(CfgReader& cfg) override;
};

#endif // HOSE369A_H
