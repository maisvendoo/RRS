#include    <ALSN-decoder.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DecoderALSN::DecoderALSN(QObject *parent) : Device(parent)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
DecoderALSN::~DecoderALSN()
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DecoderALSN::step(double t, double dt)
{
    Device::step(t, dt);

    update_timer->step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DecoderALSN::setCoilSignal(ALSN code)
{
    last_code = code;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DecoderALSN::preStep(state_vector_t &Y, double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DecoderALSN::ode_system(const state_vector_t &Y,
                             state_vector_t &dYdt,
                             double t)
{

}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DecoderALSN::load_config(CfgReader &cfg)
{
    QString secName = "Device";
    double update_interval = 5.0;

    cfg.getDouble(secName, "UpdateInterval", update_interval);

    update_timer = new Timer(update_interval, false);
    connect(update_timer, &Timer::process, this, &DecoderALSN::slotOnUpdateTimer);

    update_timer->start();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void DecoderALSN::slotOnUpdateTimer()
{
    // Полученный с катушки код отличается от текущего
    if (current_code != last_code)
    {
        for (size_t i = 0; i < recv_codes.size() - 1; ++i)
        {
            // Сдвигаем ранее принятые значения
            recv_codes[i] = recv_codes[i+1];
        }

        // Записываем в хвост массива новое значение
        *(recv_codes.end() - 1) = last_code;
    }

    if ( *(recv_codes.end() - 1) ==  *(recv_codes.end() - 2))
    {
        current_code = *(recv_codes.end() - 1);
    }
}

