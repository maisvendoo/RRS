#include    "exceptions.h"

#include    "Journal.h"
#include    <signal.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void handle_sigsegv(int signum)
{
    Journal::instance()->info(QString("FAULT by %1 (SIGSEGV)").arg(signum));
    signal(signum, SIG_DFL);
    exit(3);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void handle_sigfpe(int signum)
{
    Journal::instance()->info(QString("FAULT by %1 (SIGFPE)").arg(signum));
    signal(signum, SIG_DFL);
    exit(3);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void register_handlers()
{
    signal(SIGSEGV, handle_sigsegv);
    signal(SIGFPE, handle_sigfpe);
}
