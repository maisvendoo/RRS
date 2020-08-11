#include    "keys-control.h"
#include    "Journal.h"

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeysControl::KeysControl(QObject *parent) : QObject(parent)
    , is_started(false)
{
    keys_data.setKey("keys");

    if (!keys_data.create(1024))
    {
        if (!keys_data.attach())
        {
            Journal::instance()->error("ERROR: Can't attach to shared memory");
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
KeysControl::~KeysControl()
{

}

void KeysControl::start()
{
    if (!is_started)
    {
        this->moveToThread(&keys_thread);

        connect(&keys_thread, &QThread::started, this, &KeysControl::process);

        keys_thread.start();
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void KeysControl::process()
{
    is_started = true;

    while (1)
    {
        if (keys_data.lock())
        {
            QByteArray data(static_cast<char*>(keys_data.data()), keys_data.size());

            if (data.size() != 0)
                emit sendDataToTrain(data);

            keys_data.unlock();
        }

        QThread::sleep(10);
    }
}
