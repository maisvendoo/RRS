#include    <PlatformInput.h>

#ifdef _WIN32

#include    <windows.h>

#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool isAltPhysicallyPressed()
{
#ifdef _WIN32

    return (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

#else

    return false;

#endif
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void enforceLatinKeyboardLayout()
{
#ifdef _WIN32

    static HKL en_layout = ::LoadKeyboardLayoutA("00000409", KLF_ACTIVATE);

    if ((en_layout != nullptr) && (::GetKeyboardLayout(0) != en_layout))
    {
        ::ActivateKeyboardLayout(en_layout, KLF_SETFORPROCESS);
    }

#endif
}
