#include    <KeyPauseProcess.h>

#ifdef      __WIN32__
#include    <windows.h>
#else

#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool isPausePressed()
{
    bool key_pause_state = false;

#ifdef __WIN32__
    key_pause_state = (GetAsyncKeyState(VK_PAUSE) & 0x8000) != 0;
#else

#endif

    return key_pause_state;
}
