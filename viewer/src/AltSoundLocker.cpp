#include    <AltSoundLocker.h>

#ifdef _WIN32

#include <vsg/platform/win32/Win32_Window.h>

    // Сохраняем указатель на оригинальную WindowProc
    WNDPROC g_OriginalProc = nullptr;

    LRESULT CALLBACK SubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_MENUCHAR)
        {
            return MAKELONG(0, MNC_CLOSE); // подавляем звук
        }

        // Передаём всё остальное оригинальной процедуре
        return CallWindowProc(g_OriginalProc, hwnd, msg, wParam, lParam);
    }

#endif

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void lockAltSound(vsg::Window *window)
{
#ifdef _WIN32

    if (auto win32window = dynamic_cast<vsgWin32::Win32_Window *>(window))
    {
        HWND hWnd = *win32window;

        if (!hWnd)
        {
            return;
        }

       g_OriginalProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)SubclassProc);
    }

#endif
}
